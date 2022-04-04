
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

Piano::Application app;

// emulates the each key. Used in main()
typedef struct Key
{
    int Position;// which key on keyboard. These will be in dec instead of hex for simplicity.
    bool Pressed; //0 off, 1 on
} Key;

int i, character;
std::string logger;
std::string midiLogFile;
std::streamoff p=0;
std::ifstream ifs;

int currentkey;
int currentpressed;

Key Keyboard[61];


// Returns false if a fatal error occurs
b8 Init()
{
  app.ExecuteCommand("sudo truncate -s 0 mididata.mid");
  //app.ExecuteCommand("sudo amidi -p hw:1 --receive=mididata.mid -d");
  
  midiLogFile = "PianoGame/mididata.mid";
  ifs.open(midiLogFile.c_str());
  
  if (!ifs)
  {
    PianoLogError("File '%s' not open", midiLogFile.c_str());
  }
  
  PianoLogDebug("%s", midiLogFile.c_str());
  
  // Place test notes
  for (u32 i = 0; i < 300; i++)
  {
    app.PlaceNoteOnTimeline(i % 62, i, 1.0f);
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

  return true;
}

// Returns false if a fatal error occurs
b8 Update(float _delta)
{
  ifs.seekg(p);
  getline(ifs, logger);
  for(i=0; i<logger.size(); i++)
  {
    character = int(logger[i]);
    std::cout << std::hex << character << "\n";
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


  // Move the camera up the timeline
  Piano::Renderer::PlaceCamera(Piano::time.totalTime);

  static b8 tmpShouldClearFlag = true;
  if (tmpShouldClearFlag && Piano::time.totalTime > 5.0f)
  {
    tmpShouldClearFlag = false;
    app.ClearScreenText();
    app.ClearWorldText();
    app.PrintToScreen({}, "Text cleared from screen and world");
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
  return true;
}

int main()
{
  for(i=0; i<61; i++)
  {
      // creates all keys and sets them to off
      Keyboard[i].Position = 36+i;  // this position is in dec
      Keyboard[i].Pressed = false; 
      //stdio::cout << i << endl;
  }

  Piano::ApplicationSettings appSettings {};
  appSettings.InitFunction = Init;
  appSettings.UpdateFunction = Update;
  appSettings.ShutdownFunction = Shutdown;
  appSettings.rendererSettings.windowExtents = { 800, 600 };
  appSettings.rendererSettings.keyboardViewWidth = 63.0f;
  appSettings.rendererSettings.previewDuration = 5.0f;

  app.Run(&appSettings);

  return 0;
}
