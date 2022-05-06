
#include <piano_hero.h>
#include <logger.h>
#include <string>
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

int keyOffset = 36;
int currentkey = 0;
int currentpressed;

Key Keyboard[61];

int start_flag = 0;

int first_press = 0;

// Song file names =====
const std::vector<const char*> songFileNames {
  "star.mid",
  "ttls.mid",
  "testsongoctave.mid",
  "rugrats.mid",
  "doublenotes_slow1.mid",
  "canond.mid",
  "customSong.mid"
};

// how fast to move camera
float difficulty_mult = 1.0f;

// time keepers
float songtime = 0;
float temptime = 0;
float recordTime = 0;

// score keeping
float score = 0;

// notes from file
std::vector<Piano::note> loadedNotes;

float previewDuration = 3.0f; // number of seconds to see ahead
uint64_t keyboardState = 0;
uint64_t keyboardFrameState = 0;

u32 readInputThread = 0;

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

void* CreateSong(void* _in)
{
  printf("write thread\n");
  app.ExecuteCommand(">customSong.mid");
  
  std::string commandStr = "sudo arecordmidi -p 20:0 ";
  commandStr.append(PIANO_RESOURCE_DIR);
  commandStr.append("songs/customSong.mid");
  
  printf("Writing to '%s'\n", commandStr.c_str());
  
  app.ExecuteCommand(commandStr.c_str());
  return 0;
}

void* ReadPianoInput(void* _in)
{
  printf("READ THREAD\n");
  app.ExecuteCommand("sudo amidi -p hw:1 --receive=mididata.mid");
  
  return 0;
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
  
  // Runs for the entire execution
  readInputThread = app.CreateThread(ReadPianoInput);
  
  // Initialize game state
  TransitionToState(Main_Menu);

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
  PianoLogInfo("<> Runtime : %f", Piano::time.totalTime);
  
  app.KillThread(readInputThread);
  
  return true;
}

void UpdateInput()
{
  keyboardFrameState = 0;
  
  ifs.seekg(p);
  getline(ifs, logger);
  
  //printf("AAAAAAA : %d\n", logger.size());
  for(i=0; i<logger.size(); i++)
  {
    character = uint8_t(logger[i]);
    if((i%3) == 0)
    {
      currentpressed = character == 144;
    }
    if((i%3) == 1)
    {
      currentkey = character - keyOffset;

      Keyboard[currentkey].Pressed = currentpressed;
      
      keyboardState = currentpressed ? (keyboardState | (u64(1) << currentkey)) : (keyboardState & ~(u64(1) << currentkey));
      keyboardFrameState = currentpressed ? (keyboardState | (u64(1) << currentkey)) : (keyboardState & ~(u64(1) << currentkey));
    }
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

// ====================
// Transition
// ====================

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
      score = 0;
    } break;  
    case Record:
    {
      CurrentStateUpdate = StateUpdateRecord;
    } break;  
    case Prerecord:
    {
      CurrentStateUpdate = StateUpdatePrerecord;
    } break;
    case Song_Menu:
    {
      ShowSongSelection();
      
      
      CurrentStateUpdate = StateUpdateSongMenu;
    } break;
  }
  
  currentState = _newState;
}

void ShowSongSelection()
{
  Piano::Renderer::PlaceCamera(Piano::time.totalTime * 0.0f);
  Piano::TextPrintSettings printSettings {};

  printSettings.color = { 255.0f, 255.0f, 255.0f };
  printSettings.startPosition = { 800.0f, 700.0f };
  app.PrintToScreen(printSettings, "Select A Song");

  int index = 1;
  char buffer[256];
  
  for (const char* n : songFileNames)
  {
    sprintf(buffer, "%d : %s", index - 1, n);
    printSettings.color = { 255.0f / index, 255.0f / index, 255.0f / index };
    printSettings.startPosition = { 800.0f, 700.0f - (50.0f * index++) };
    app.PrintToScreen(printSettings, buffer);
    
    printSettings.startPosition = {(float)index - 1.0f, 0.0f};
    printSettings.scale = 1.5f;
    app.PrintToWorld(printSettings, "%d", index - 1);
    
    printSettings.scale = 1.0f;
  }

  Piano::Renderer::PlaceCamera(0);
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

void ShowMainMenu()
{
  Piano::Renderer::PlaceCamera(Piano::time.totalTime * 0.0f);
  Piano::TextPrintSettings printSettings {};
  
  printSettings.color = { 255.0f, 255.0f, 255.0f };
  printSettings.startPosition = { 800.0f, 500.0f };
  app.PrintToScreen(printSettings, "Song Selection");
  printSettings.color = { 255.0f, 255.0f, 0.0f };
  printSettings.startPosition = { 800.0f, 400.0f };
  app.PrintToScreen(printSettings, "Song Creation");
}


void StateUpdateMainMenu(float _delta)
{
  if (keyboardFrameState)
  {
    if (currentkey == 0)
    {
      TransitionToState(Song_Menu);
    }
    if (currentkey == 1)
    {
      TransitionToState(Prerecord);
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
  
  static float noteStart = 0.0f;
  static float noteEnd = 0.0f;  
  
  // Scoring =====
  for(u64 i = 0; i < loadedNotes.size(); i++)
  {
    noteStart = loadedNotes[i].startTime;
    noteEnd = loadedNotes[i].startTime + loadedNotes[i].duration;
    
    for (u64 tmpState = keyboardState; tmpState; tmpState &= ~(tmpState & -tmpState))
    {
      score += (100 * _delta) * (songtime >= noteStart && songtime <= noteEnd) * ((u32)loadedNotes[i].keyPosition == BitScanForward(tmpState) + 36);
    }
  }

  // printing score to screen
  Piano::TextPrintSettings printSettings {};
  
  app.ClearScreenText();
  printSettings.color = { 0.0f, 0.0f, 0.0f };
  printSettings.startPosition = { 950.0f, 995.0f };
  app.PrintToScreen(printSettings, "Score: %7.2f", score);
  printSettings.color = { 224.0f, 224.0f, 224.0f };
  printSettings.startPosition = { 960.0f, 1000.0f };
  app.PrintToScreen(printSettings, "Score: %7.2f", score);
  
  
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

void StateUpdateRecord(float _delta)
{
  // once timer ends, stop recording and go back to main menu
  if(recordTime > 0.0f)
  {
    recordTime -= _delta;
  }
  else
  {
    app.KillThread(readInputThread);
    readInputThread = app.CreateThread(ReadPianoInput);
    TransitionToState(Main_Menu);
  }

}

void StateUpdatePrerecord(float _delta)
{
  if(keyboardFrameState)
  {
    recordTime = 5;
    app.KillThread(readInputThread);
    readInputThread = app.CreateThread(CreateSong);
    TransitionToState(Record);
  }
}  

void StateUpdateSongMenu(float _delta)
{
  if (keyboardFrameState)
  {
    // Load the selected song (Don't display it yet)
    
    if (currentkey < songFileNames.size())
    {
      loadedNotes = LoadNotesFromFile(songFileNames[currentkey]);
      TransitionToState(Difficulty_Select);
      
      for (const Piano::note& n : loadedNotes)
      {
        printf("key number: %f\n", n.keyPosition);
      }
    }
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
