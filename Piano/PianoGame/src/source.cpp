
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

// Song file names =====
const std::vector<const char*> songFileNames {
  "ttls.mid",
  "testsongoctave.mid"
};

// how fast to move camera
float difficulty_mult = 1.0f;

// time keepers
float songtime = 0;
float temptime = 0;

// score keeping
int score = 0;

// notes from file
std::vector<Piano::note> loadedNotes;

float previewDuration = 3.0f; // number of seconds to see ahead
uint64_t keyboardState = 0;
uint64_t keyboardFrameState = 0;

// functions

void(*CurrentStateUpdate)(float _delta);
GameStatesEnum currentState;

int BitScanForward(u64 _input)
{
  int bitIndex = 0;
  _input = _input & -_input;
  bitIndex += ((0xffffffff00000000 & _input) != 0) * 32;
  bitIndex += ((0xffff0000ffff0000 & _input) != 0) * 16;
  bitIndex += ((0xff00ff00ff00ff00 & _input) != 0) * 8;
  bitIndex += ((0xf0f0f0f0f0f0f0f0 & _input) != 0) * 4;
  bitIndex += ((0xcccccccccccccccc & _input) != 0) * 2;
  bitIndex += ((0xaaaaaaaaaaaaaaaa & _input) != 0) * 1;
  return bitIndex;
}
int BitScanForward(u32 _input)
{
  return BitScanForward((u64)_input);
}

void ReadPianoInput()
{
  app.ExecuteCommand("sudo amidi -p hw:1 --receive=mididata.mid");
}

// Returns false if a fatal error occurs
b8 Init()
{
  app.ExecuteCommand("sudo truncate -s 0 mididata.mid");
  
  u32 testThingy = (1 << 2) | (1 << 15) | (1 << 28);
  printf("A : %d", BitScanForward(testThingy));
  testThingy &= ~(testThingy & -testThingy);
  printf("B : %d", BitScanForward(testThingy));
  testThingy &= ~(testThingy & -testThingy);
  printf("C : %d", BitScanForward(testThingy));
  testThingy &= ~(testThingy & -testThingy);
  printf("D : %d", BitScanForward(testThingy));
  
  
  
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

void UpdateInput()
{
  keyboardFrameState = 0;
  
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
      keyboardFrameState = currentpressed ? (keyboardState | (u64(1) << currentkey)) : (keyboardState & ~(u64(1) << currentkey));
      
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
      songtime = 0;
      // Start updating
      for (auto& n : loadedNotes)
      {
        app.PlaceNoteOnTimeline(n.keyPosition, n.startTime, n.duration);
      }
      app.PushNotesTimelineToRenderer();
      app.SetRenderOctaveLines(true);
      
      CurrentStateUpdate = StateUpdatePlaying;
    } break;
    case Preplay:
    {
      songtime = 0;
      // Load notes for render
      for (auto& n : loadedNotes)
      {
        app.PlaceNoteOnTimeline(n.keyPosition, n.startTime, n.duration);
      }
      app.PushNotesTimelineToRenderer();
      app.SetRenderOctaveLines(true);
      
      Piano::Renderer::PlaceCamera(0);
      CurrentStateUpdate = StateUpdatePreplay;
    } break;  
  
  }
  
  currentState = _newState;
}

void ShowMainMenu()
{
  Piano::Renderer::PlaceCamera(Piano::time.totalTime * 0.0f);
  Piano::TextPrintSettings printSettings {};

  printSettings.color = { 255.0f, 255.0f, 255.0f };
  printSettings.startPosition = { 800.0f, 700.0f };
  app.PrintToScreen(printSettings, "Select A Song");

  int index = 1;
  for (const char* n : songFileNames)
  {
    printSettings.color = { 255.0f / index, 255.0f / index, 255.0f / index };
    printSettings.startPosition = { 800.0f, 700.0f - (50.0f * index++) };
    app.PrintToScreen(printSettings, n);
  }
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
  if (keyboardFrameState)
  {
    // Load the selected song (Don't display it yet)
    
    if (currentkey < songFileNames.size())
    {
      loadedNotes = LoadNotesFromFile(songFileNames[currentkey]);
      TransitionToState(Difficulty_Select);
      
      int index = 0;
      for (const Piano::note& n : loadedNotes)
      {
        printf("Note %d : %f\n", index++, n.keyPosition);
      }
    }
  }
}

void StateUpdateDifficultySelect(float _delta)
{
  if (keyboardFrameState)
  {
    if (currentkey == 2)
    {
      difficulty_mult = 0.5f;
      TransitionToState(Preplay);
    }
    if (currentkey == 4) {
      difficulty_mult = 0.7f;
      TransitionToState(Preplay);
    }
    if (currentkey == 5) {
      difficulty_mult = 1.0f;
      TransitionToState(Preplay);
    }
  }
}

void StateUpdatePlaying(float _delta)
{
  // Move the camera up the timeline
  // songtime keeps track of the ingame realtime since it changes with the difficulty
  songtime += _delta * difficulty_mult; // this might need some adjusting
  Piano::Renderer::PlaceCamera(songtime);
  
  static f32 localScore = 0.0f;
  
  static float noteStart = 0.0f;
  static float noteEnd = 0.0f;
  
  int currentKeyIndex = 0;
  
  // Scoring =====
  for(u64 i = 0; i < loadedNotes.size(); i++)
  {
    noteStart = loadedNotes[i].startTime;
    noteEnd = loadedNotes[i].startTime + loadedNotes[i].duration;
    
    if (songtime >= noteStart && songtime <= noteEnd)
      currentKeyIndex = i;
    
    for (u64 tmpState = keyboardState; tmpState; tmpState &= ~(tmpState & -tmpState))
    {
      localScore += (10 * _delta) * (songtime >= noteStart && songtime <= noteEnd) * ((u32)loadedNotes[i].keyPosition == BitScanForward(tmpState) + 36);
    }
  }

  // printing score to screen
  Piano::TextPrintSettings printSettings {};
  printSettings.color = { 224.0f, 224.0f, 224.0f };
  printSettings.startPosition = { 960.0f, 1000.0f };

  app.ClearScreenText();
  app.PrintToScreen(printSettings, "Score: %f -- %d", localScore, BitScanForward(keyboardState) + 36);
  
  
  if (songtime >= (loadedNotes[loadedNotes.size() - 1].startTime + loadedNotes[loadedNotes.size() - 1].duration + 1.0f))
  {
    TransitionToState(Main_Menu);
  }
  
}

void StateUpdatePreplay(float _delta)
{
  if(keyboardFrameState)
  {
    TransitionToState(Playing);
  }
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
