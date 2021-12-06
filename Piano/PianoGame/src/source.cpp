
#include <piano_hero.h>

#include <core/global_stats.h>

#include <stdio.h>
#include <random>

PianoApplication app;

// Return false if a fatal error occurs
void Init()
{
  printf("TMP game Init\n");

  app.PlaceNoteOnTimeline(0, 0.0f, 1.0f);
  app.PlaceNoteOnTimeline(1, 0.5f, 1.0f);
}

void Update(float _delta)
{
  static float timer = 0;
  const float NoteFrequency = 1.0f * 1000.0f;

  // Random float between 0.5 & 2.0
  float duration = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 1.5f + 0.5f;
  u32 note = rand() % 10;

  if (timer >= NoteFrequency)
  {
    app.PlaceNoteOnTimeline(note, globalStats.realTime + 1.0f, duration);
    timer = 0;
  }

  timer += _delta;
}

void Shutdown()
{
  printf("TMP game Shutdown\n");
}

int main()
{
  phApplicationSettings appSettings {};
  appSettings.Initialize = Init;
  appSettings.Update = Update;
  appSettings.Shutdown = Shutdown;
  appSettings.windowExtents = { 800, 600 };

  app.Run(&appSettings);

  return 0;
}
