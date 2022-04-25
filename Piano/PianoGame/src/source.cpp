
#include <piano_hero.h>
#include <logger.h>

#include <math.h>
#include <stdio.h>
#include <random>
#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <cstdio>
#include <stdlib.h>
#include <set>
#include <stdint.h>
#include <bitset>

#include "midi.h"
#include "gameplay.h"

Piano::Application app;

// emulates the each key. Used in main()
typedef struct Key
{
    bool Pressed; //0 off, 1 on
} Key;

int i;
uint8_t character;
std::string logger;
std::string midiLogFile;
std::streamoff p=0;
std::ifstream ifs;

int currentkey = 150;
int currentpressed;

Key Keyboard[61];

int start_flag = 0;

int first_press = 0;

// file name
const char* fileName = "ttls.mid";

// how fast to move camera
float difficulty_mult = 1.0f;

// time keepers
float realtime = 0;
float temptime = 0;

// score keeping
int score = 0;

// notes from file
std::vector<Piano::note> loadedNotes;

float previewDuration = 3.0f; // number of seconds to see ahead
uint64_t keyboardState = 0;

// functions

void(*CurrentStateUpdate)(float _delta);
GameStatesEnum currentState;

void ReadPianoInput()
{
  app.ExecuteCommand("sudo amidi -p hw:1 --receive=mididata.mid");
}

// Returns false if a fatal error occurs
b8 Init()
{
  app.ExecuteCommand("sudo truncate -s 0 mididata.mid");
  
  midiLogFile = "mididata.mid";
  ifs.open(midiLogFile.c_str());
  
  if (!ifs)
  {
    PianoLogError("File '%s' not open", midiLogFile.c_str());
    return false;
  }
  
  PianoLogDebug("%s", midiLogFile.c_str());
  
  // Initialize game state
  TransitionToState(Main_Menu);
  
  // Runs for the entire execution
  Piano::Platform::StartThread(ReadPianoInput);

  return true;
}

// Returns false if a fatal error occurs
b8 Update(float _delta)
{
  UpdateInput();

  CurrentStateUpdate(_delta);

  #ifdef PIANO_DEBUG
  // check which notes are pressed and how many are pressed
  for(u64 i = 0; i < 61; i++) {
    if(Keyboard[i].Pressed == true) {
      //testing stuff
//      printf("score: %d,  realtime: %f\n", score, realtime);
      
      for(u64 j = 0; j<loadedNotes.size(); j++) {
        if((realtime >= loadedNotes[j].startTime) && (realtime <= (loadedNotes[j].startTime + loadedNotes[j].duration)) && (i == loadedNotes[j].keyPosition)) {
          score = score + 10;
//          printf("score: %d\n", score);
        }
      }
    }
  }
  
  // Log the average delta time & framerate =====
  static f32 deltasSum = 0.0f;
  static u32 deltasCount = 0;
  static const f32 fpsPrintFrequency = 1.0f; // Seconds

  deltasSum += _delta;
  deltasCount++;

  if (deltasSum >= fpsPrintFrequency)
  {
    double avg = deltasSum / deltasCount;
    PianoLogInfo("\n<> %4.3f ms -- %4.0f fps -- %f", avg * 1000, 1 / avg, Piano::time.totalTime);

    deltasSum = 0;
    deltasCount = 0;
  }
  #endif // PIANO_DEBUG

  return true;
}

// Returns false if a fatal error occurs
b8 Shutdown()
{
  for(i = 0; i<loadedNotes.size(); i++){
      printf("%f,  %f,  %f\n", loadedNotes[i].keyPosition, loadedNotes[i].startTime, loadedNotes[i].duration);
  }
  PianoLogInfo("<> Runtime : %f", Piano::time.totalTime);
  PianoLogInfo("Piano inpt command is still running. Hit ctrl+%c", 'C');
  return true;
}

int main()
{
  for(i=0; i<61; i++)
  {
      // creates all keys and sets them to off
      Keyboard[i].Pressed = false; // add 36 to i value to use as a note number
  }

  Piano::ApplicationSettings appSettings {};
  appSettings.InitFunction = Init;
  appSettings.UpdateFunction = Update;
  appSettings.ShutdownFunction = Shutdown;
  appSettings.rendererSettings.windowExtents = { 800, 600 };
  appSettings.rendererSettings.keyboardViewWidth = 36.0f;
  appSettings.rendererSettings.previewDuration = previewDuration;
  appSettings.rendererSettings.fullscreen = true;

  app.Run(&appSettings);

  return 0;
}

void UpdateInput()
{
  ifs.seekg(p);
  getline(ifs, logger);
  for(i=0; i<logger.size(); i++)
  {
    character = uint8_t(logger[i]);
    if((i%3) == 0)
    {
      currentpressed = character == 144;
    }
    if((i%3) == 1)
    {
      currentkey = character-36;
      
//      PianoLogInfo("%u", currentkey);
      
      Keyboard[currentkey].Pressed = currentpressed;
      
      keyboardState = currentpressed ? (keyboardState | (u64(1) << currentkey)) : (keyboardState & ~(u64(1) << currentkey));
      
      std::bitset<64> tmp(keyboardState);
//      std::cout << tmp << "\n";
    }
  
//    PianoLogDebug("%u", character);
  }

  if(ifs.tellg() == -1)
  {
    p = p + logger.size();
  }
  else
  {
    p = ifs.tellg();
  }
  ifs.clear();
}

void TransitionToState(GameStatesEnum _newState)
{
  // Clear
  app.ClearNotesTimeline();
  app.ClearScreenText();
  app.ClearWorldText();

  switch (_newState)
  {
    case Main_Menu:
    { 
      app.SetRenderOctaveLines(false);
      ShowMainMenu();
      CurrentStateUpdate = StateUpdateMainMenu;
    } break;
    case Difficulty_Select:
    {
      app.SetRenderOctaveLines(false);
      ShowDifficultySelection();
      CurrentStateUpdate = StateUpdateDifficultySelect;
    } break;
    case Playing:
    {
      // Load notes for render
      for (auto& n : loadedNotes)
      {
        app.PlaceNoteOnTimeline(n.keyPosition, n.startTime, n.duration);
      }
      app.PushNotesTimelineToRenderer();
      app.SetRenderOctaveLines(true);
      // Start updating
      CurrentStateUpdate = StateUpdatePlaying;
      realtime = -(previewDuration * difficulty_mult);
    } break;
  }
  
  currentState = _newState;
}

void ShowMainMenu()
{
  Piano::Renderer::PlaceCamera(Piano::time.totalTime * 0.0f);
  Piano::TextPrintSettings printSettings {};
  
  printSettings.color = { 255.0f, 255.0f, 255.0f };
  printSettings.startPosition = { 800.0f, 500.0f };
  app.PrintToScreen(printSettings, "Select A Song");
  printSettings.color = { 255.0f, 255.0f, 0.0f };
  printSettings.startPosition = { 800.0f, 400.0f };
  app.PrintToScreen(printSettings, "Twinkle Twinkle Little Star");
  printSettings.color = { 224.0f, 0.0f, 0.0f };
  printSettings.startPosition = { 800.0f, 300.0f };
  app.PrintToScreen(printSettings, "Song B");
}

void ShowDifficultySelection()
{
  Piano::Renderer::PlaceCamera(Piano::time.totalTime * 0.0f);
  
  Piano::TextPrintSettings printSettings {};
  printSettings.color = { 255.0f, 255.0f, 255.0f };
  printSettings.startPosition = { 800.0f, 500.0f };
  app.PrintToScreen(printSettings, "Easy");
  printSettings.color = { 255.0f, 255.0f, 0.0f };
  printSettings.startPosition = { 800.0f, 400.0f };
  app.PrintToScreen(printSettings, "Medium");
  printSettings.color = { 224.0f, 0.0f, 0.0f };
  printSettings.startPosition = { 800.0f, 300.0f };
  app.PrintToScreen(printSettings, "Hard");

  start_flag = 1;
  currentkey = 150;
}

void StateUpdateMainMenu(float _delta)
{
  //Load the song onto the screen after first key pressed
  if (currentkey == 0) {
    app.PrintToScreen({}, "Key was pressed! %d");
    
    // Load the selected song (Don't display it yet)
    loadedNotes = LoadNotesFromFile(fileName);
    TransitionToState(Difficulty_Select);
  }
}

void StateUpdateDifficultySelect(float _delta)
{
  // make variable for currentkey == ###
  //easy press key number 2
  if (currentkey == 2)
  {
    difficulty_mult = 0.1f;
    TransitionToState(Playing);
    app.PrintToScreen({}, "Key was pressed! %d");
  }
  //Medium press key number 4
  if (currentkey == 4) {
    difficulty_mult = 0.3f;
    TransitionToState(Playing);
    app.PrintToScreen({}, "Key was pressed! %d");
  }
  //hard press key number 5
  if (currentkey == 5) {
    difficulty_mult = 0.5f;
    TransitionToState(Playing);
    app.PrintToScreen({}, "Key was pressed! %d");
  }
}

void StateUpdatePlaying(float _delta)
{
  // Move the camera up the timeline
  // realtime keeps track of the ingame realtime since it changes with the difficulty
  realtime += _delta * difficulty_mult; // this might need some adjusting
  Piano::Renderer::PlaceCamera(realtime);

  // printing score to screen
  Piano::TextPrintSettings printSettings {};
  printSettings.color = { 224.0f, 224.0f, 224.0f };
  printSettings.startPosition = { 960.0f, 1000.0f };

  app.ClearScreenText();
  app.PrintToScreen(printSettings, "Score: %d", score);
}
