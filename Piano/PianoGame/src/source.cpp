
#include <piano_hero.h>
#include <logger.h>

#include <math.h>
#include <stdio.h>
#include <random>

Piano::Application app;

// Returns false if a fatal error occurs
b8 Init()
{
  // Place test notes
  for (u32 i = 0; i < 300; i++)
  {
    app.PlaceNoteOnTimeline(i % 62, i, 1.0f);
  }
  app.PushNotesTimelineToRenderer();

  return true;
}

// Returns false if a fatal error occurs
b8 Update(float _delta)
{
  // Move the camera up the timeline
  //  Should probably change this to a boolean flag in application
  Piano::Renderer::PlaceCamera(Piano::time.totalTime);

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
    PianoLogInfo("<> %4.3f ms -- %4.0f fps", avg * 1000, 1 / avg);

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
  Piano::ApplicationSettings appSettings {};
  appSettings.InitFunction = Init;
  appSettings.UpdateFunction = Update;
  appSettings.ShutdownFunction = Shutdown;
  appSettings.rendererSettings.windowExtents = { 800, 600 };
  appSettings.rendererSettings.keyboardViewWidth = 63.0f;
  appSettings.rendererSettings.previewDuration = 63.0f;

  app.Run(&appSettings);

  return 0;
}
