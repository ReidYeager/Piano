
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

int currentkey;
int currentpressed;

Key Keyboard[61];


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
  }
  
  PianoLogDebug("%s", midiLogFile.c_str());
  
  float time = 0.0f;

  // Place test notes
  //const char* fileName = "doublenotes_slow1.mid"; // Does not read notes properly for some reason
  //const char* fileName = "testsongoctave.mid";
  //const char* fileName = "rugrats.mid"; // Does not read notes properly for some reason
  const char* fileName = "ttls.mid";
  //const char* fileName = "canond.mid";

  std::vector<Piano::note> loadedNotes = LoadNotesFromFile(fileName);
  for (auto& n : loadedNotes)
  {
    app.PlaceNoteOnTimeline(n.keyPosition, n.startTime, n.duration);
  }

  app.PushNotesTimelineToRenderer();

  app.PrintToScreen({}, "This is some test text with %d", 1221);
  app.PrintToWorld({{25.0f, 3.0f}}, "Text in the world %f", 99.012f);

  Piano::TextPrintSettings printSettings {};
  printSettings.color = { 0.8f, 0.1f, 0.8f };
  printSettings.startPosition = { 250.0f, 500.0f };
  app.PrintToScreen(printSettings, "Wow even text over here. %c", '!');
  printSettings.color = { 0.1f, 0.75f, 0.15f };
  printSettings.startPosition = { 2.0f, 8.0f };
  app.PrintToWorld(printSettings, "AAAAAAAA %s", "AA");

  // Runs for the entire execution
  Piano::Platform::StartThread(ReadPianoInput);

  return true;
}
uint64_t keyboardState = 0;

// Returns false if a fatal error occurs
b8 Update(float _delta)
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
      
      PianoLogInfo("%u", currentkey);
      
      Keyboard[currentkey].Pressed = currentpressed;
      
      keyboardState = currentpressed ? (keyboardState | (u64(1) << currentkey)) : (keyboardState & ~(u64(1) << currentkey));
      
      std::bitset<64> tmp(keyboardState);
      std::cout << tmp << "\n";
    }
  
    PianoLogDebug("%u", character);
  }
  
  //for (u64 i = keyboardState; i;)
  //{
  //  u64 lowestBit = i & -i;
  //  u64 index = 0;
  //  index += ((lowestBit & 0xffffffff00000000) != 0) * 32;
  //  index += ((lowestBit & 0xffff0000ffff0000) != 0) * 16;
  //  index += ((lowestBit & 0xff00ff00ff00ff00) != 0) * 8;
  //  index += ((lowestBit & 0xf0f0f0f0f0f0f0f0) != 0) * 4;
  //  index += ((lowestBit & 0xcccccccccccccccc) != 0) * 2;
  //  index += ((lowestBit & 0xaaaaaaaaaaaaaaaa) != 0) * 1;
  //  
  //  app.PlaceNoteOnTimeline(index + 36, Piano::time.totalTime + 3.0f, _delta * 1.5f);
  //  
  //  i -= lowestBit;
  //}
  //if (keyboardState)
  //{
  //  app.PushNotesTimelineToRenderer();
  //}

  if(ifs.tellg() == -1)
  {
    p = p + logger.size();
  }
  else
  {
    p = ifs.tellg();
  }
  ifs.clear();


  // Move the camera up the timeline
  Piano::Renderer::PlaceCamera(Piano::time.totalTime * 0.5f);

  static b8 tmpShouldClearFlag = true;
  if (tmpShouldClearFlag && Piano::time.totalTime > 5.0f)
  {
    tmpShouldClearFlag = false;
    app.ClearScreenText();
    app.ClearWorldText();
    app.PrintToScreen({}, "Text cleared from screen and world");
    //app.SetRenderOctaveLines(false);
  }

  #ifdef PIANO_DEBUG
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
  appSettings.rendererSettings.previewDuration = 3.0f;
  appSettings.rendererSettings.fullscreen = true;

  app.Run(&appSettings);

  return 0;
}
