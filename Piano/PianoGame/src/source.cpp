
#include <stdio.h>

#include <piano_hero.h>

// Return false if a fatal error occurs
void Init()
{
  printf("TMP game Init\n");
}

void Update(double)
{
  //printf("TMP game Update\n");
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

  PianoApplication app;
  app.Run(&appSettings);

  return 0;
}
