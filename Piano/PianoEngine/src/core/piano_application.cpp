
#include "defines.h"

#include "core/piano_application.h"
#include "core/global_stats.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <chrono>
#include <math.h>

void PianoApplication::Initialize(phApplicationSettings* _settings)
{
  pianoPlatform.Initialize(800, 600);
  pianoRenderer.Initialize(&pianoPlatform);

  context.ClientInitialize = _settings->Initialize;
  context.ClientUpdate = _settings->Update;
  context.ClientShutdown = _settings->Shutdown;

  context.ClientInitialize();
}

void PianoApplication::Shutdown()
{
  context.ClientShutdown();

  pianoRenderer.Shutdown();
  pianoPlatform.Shutdown();
}

void PianoApplication::HandleInput(GLFWwindow* _window)
{
  if (glfwGetKey(_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(_window, true);
}

void PianoApplication::Run(phApplicationSettings* _settings)
{
  try
  {
    Initialize(_settings);
    MainLoop();
    Shutdown();
  }
  catch (const char* e)
  {
    printf("PianoHero bailed out with an error of : %s", e);
    return;
  }

  glfwTerminate();
}

void PianoApplication::PlaceNoteOnTimeline(u32 _note, float _startTime, float _duration)
{
  // Calculate scene position
}

UiElement PianoApplication::AddUiElement(vec2 _position, vec2 _scale /*= {1, 1}*/)
{
  // Calculate clip-space position

  return {};
}

// =======================
// Main Loop
// =======================

void PianoApplication::MainLoop()
{
  auto start = std::chrono::steady_clock::now();
  auto end = std::chrono::steady_clock::now();
  auto deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

  glm::mat4 mvp;
  float pianoTime = 0.0f;

  double deltasSum = 0.0f;
  int deltasCount = 0;
  float fpsPrintFrequency = 1.0f * 1000.0f; // Milliseconds

  while (!pianoPlatform.ShouldClose())
  {
    // Frame initializtation =====
    {
      start = end;
      HandleInput(pianoPlatform.GetWindow());
    }

    // Client loop =====
    {
      mvp = glm::translate(glm::mat4(1), glm::vec3(std::sin(globalStats.realTime), 0.0f, 0.0f));
      context.ClientUpdate(globalStats.deltaTime);
    }

    // Rendering =====
    {
      pianoRenderer.RenderFrame(mvp);

      glfwSwapBuffers(pianoPlatform.GetWindow());
      glfwPollEvents();
    }

    // Calculate times =====
    {
      end = std::chrono::steady_clock::now();
      deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

      globalStats.deltaTime = deltaTime.count() * 0.001; // uSec -> mSec
      globalStats.realTime += globalStats.deltaTime * 0.001; // mSec -> Sec

      deltasSum += globalStats.deltaTime;
      deltasCount++;

      if (deltasSum >= fpsPrintFrequency)
      {
        double avg = deltasSum / deltasCount;
        printf(" %8.0f ms -- %4.0f fps\n", avg, 1000 / avg);

        deltasSum = 0;
        deltasCount = 0;
      }
    }
  }
}
