
#include "defines.h"

#include "core/piano.h"

int main()
{
  try {
    PianoApplication app;
    app.Run();
  }
  catch (int e)
  {
    glfwTerminate();
    return e;
  }

  glfwTerminate();
  return 0;
}

