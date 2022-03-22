
#include <piano_hero.h>
#include <logger.h>

#include <math.h>
#include <stdio.h>
#include <random>

Piano::Application app;

// Return false if a fatal error occurs
void Init()
{
  PianoLogInfo("TMP game Init");

  app.PlaceNoteOnTimeline(0, 0.0f, 1.0f);
  app.PlaceNoteOnTimeline(1, 0.5f, 1.0f);
}

void Update(float _delta)
{
  Piano::Renderer::PlaceCamera(std::sin(Piano::time.totalTime));

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
    PianoLogInfo("<> %8.0f ms -- %4.0f fps", avg, 1000 / avg);

    deltasSum = 0;
    deltasCount = 0;
  }
  #endif // PIANO_DEBUG
}

void Shutdown()
{
  PianoLogInfo("TMP game Shutdown");
}

int main()
{
  Piano::ApplicationSettings appSettings {};
  appSettings.Initialize = Init;
  appSettings.Update = Update;
  appSettings.Shutdown = Shutdown;
  appSettings.rendererSettings.windowExtents = { 800, 600 };
  appSettings.rendererSettings.camera.keyboardLayout = Piano::Renderer::Keyboard_Full;
  appSettings.rendererSettings.camera.previewLength = 2.0f;

  app.Run(&appSettings);

  return 0;
}
