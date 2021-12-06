
#include "defines.h"

#include "core/piano_application.h"
#include "core/global_stats.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <chrono>
#include <math.h>
#include <vector>

// =======================
// Engine Initialization
// =======================

std::vector<glm::mat4> quadTransforms;
const u32 quadCount = 10;
u32 quadIndex = 0;

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

void PianoApplication::Initialize(phApplicationSettings* _settings)
{
  platform.Initialize(_settings->windowExtents);
  renderer.Initialize(_settings->windowExtents);

  context.ClientInitialize = _settings->Initialize;
  context.ClientUpdate = _settings->Update;
  context.ClientShutdown = _settings->Shutdown;

  context.windowExtents = _settings->windowExtents;

  quadTransforms.resize(quadCount);

  context.ClientInitialize();
}

void PianoApplication::Shutdown()
{
  context.ClientShutdown();

  renderer.Shutdown();
  platform.Shutdown();
}

void PianoApplication::HandleInput(GLFWwindow* _window)
{
  if (glfwGetKey(_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(_window, true);

  // TODO : Update the status of all applicable keys (keyboard & piano)
}

void PianoApplication::MainLoop()
{
  auto start = std::chrono::steady_clock::now();
  auto end = std::chrono::steady_clock::now();
  auto deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

  f32 deltasSum = 0.0f;
  u32 deltasCount = 0;
  f32 fpsPrintFrequency = 1.0f * 1000.0f; // Milliseconds

  // TMP
  glm::mat4 mvp;

  while (!platform.ShouldClose())
  {
    // Frame initializtation =====
    {
      start = end;
      HandleInput(platform.GetWindow());
    }

    // Client loop =====
    {
      Update();
      context.ClientUpdate(globalStats.deltaTime);
    }

    // Rendering =====
    {
      renderer.RenderFrame(quadTransforms);

      glfwSwapBuffers(platform.GetWindow());
      glfwPollEvents();
    }

    // Calculate times =====
    {
      end = std::chrono::steady_clock::now();
      deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

      globalStats.deltaTime = deltaTime.count() * 0.001f; // uSec -> mSec
      globalStats.realTime += globalStats.deltaTime * 0.001f; // mSec -> Sec

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

// =======================
// Functionality
// =======================

std::vector<glm::mat4> noteWorldTransforms;

void PianoApplication::Update()
{
  u32 i = 0;

  printf("%f\t\t", globalStats.deltaTime);

  // Update quad transforms ===== NOT ONLY FOR NOTES
  for (auto& mvp : quadTransforms)
  {
    // Directly modify the quad's Y position
    // Can't use glm::translate because it is affected by the quad's scale
    mvp[3][1] -= globalStats.deltaTime * 0.001f;

    printf("%d:%f -- ", i++, mvp[3][1]);
  }
  printf("\n");
}

void PianoApplication::PlaceNoteOnTimeline(u32 _note, f32 _startTime, f32 _duration)
{
  // Calculate note's world position =====
  f32 worldX = ((_note * 80.0f) - context.windowExtents.width * 0.45f) / (context.windowExtents.width * 0.5f);
  f32 worldY = _startTime;

  // Place and scale the note =====
  glm::mat4& mvp = quadTransforms[quadIndex];
  mvp = glm::translate(glm::mat4(1), glm::vec3(worldX, worldY, 0.0f));
  mvp = glm::scale(mvp, glm::vec3(0.1f, _duration, 1.0f));

  quadIndex = (quadIndex + 1) % quadCount;
}

UiElement PianoApplication::AddUiElement(vec2 _position, vec2 _scale /*= {1, 1}*/)
{
  // Calculate clip-space position

  return {};
}
