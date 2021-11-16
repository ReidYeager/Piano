
#include "defines.h"

#include "core/piano.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <chrono>
#include <math.h>

void PianoApplication::Loop()
{
  auto start = std::chrono::steady_clock::now();
  auto end = std::chrono::steady_clock::now();
  auto deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

  glm::mat4 mvp;
  float pianoTime = 0.0f;

  float deltasSum = 0.0f;
  int deltasCount = 0;
  float fpsPrintFrequency = 1.0f;  // Seconds
  fpsPrintFrequency *= 1000000.0f; // To microseconds;

  while (!platform.ShouldClose())
  {
    start = end;

    HandleInput(platform.GetWindow());

    pianoTime += 0.01f;

    mvp = glm::translate(glm::mat4(1), glm::vec3(std::sin(pianoTime), 0.0f, 0.0f));

    renderer.RenderFrame(mvp);
    glfwSwapBuffers(platform.GetWindow());
    glfwPollEvents();

    // Calculate times
    end = std::chrono::steady_clock::now();
    deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    deltasSum += deltaTime.count();
    deltasCount++;

    if (deltasSum >= fpsPrintFrequency)
    {
      float avg = deltasSum / deltasCount;
      printf(" %8.0f us -- %4.0f fps\n", avg, 1000000 / avg);

      deltasSum = 0;
      deltasCount = 0;
    }
  }
}

void PianoApplication::Initialize()
{
  platform.Initialize(800, 600);

  renderer.Initialize(&platform);
}

void PianoApplication::Shutdown()
{
  renderer.Shutdown();
}

void PianoApplication::HandleInput(GLFWwindow* _window)
{
  if (glfwGetKey(_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(_window, true);
}

void PianoApplication::Run()
{
  Initialize();
  Loop();
  Shutdown();
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
