
#include "defines.h"
#include "logger.h"

#include "core/piano_application.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <chrono>
#include <math.h>
#include <vector>
#include <stdio.h>

// Setup time =====
Piano::TimeInfo Piano::time;
std::chrono::steady_clock::time_point realtimeStart, frameStart, frameEnd;
const float microToSecond = 0.000001f;

void Piano::Application::Run(ApplicationSettings* _settings)
{
  try
  {
    Initialize(_settings);
    MainLoop();
    Shutdown();
  }
  catch (const char* e)
  {
    PianoLogFatal("PianoHero bailed out with an error of : %s", e);
    return;
  }

  glfwTerminate();
}

// ==========
// Init & Shutdown
// ==========

void Piano::Application::Initialize(ApplicationSettings* _settings)
{
  Piano::Platform::Initialize(_settings->rendererSettings.windowExtents);
  Piano::Renderer::Initialize(_settings->rendererSettings);

  // Initialize time =====
  {
    Piano::time.deltaTime = 0.0f;
    Piano::time.totalTime = 0.0f;
    Piano::time.frameCount = 0;

    realtimeStart = std::chrono::steady_clock::now();
    frameStart = frameEnd = realtimeStart;

    Piano::time.deltaTime = 0.0f;
  }

  // Intialize Client =====
  context.ClientInitialize = _settings->Initialize;
  context.ClientUpdate = _settings->Update;
  context.ClientShutdown = _settings->Shutdown;

  context.windowExtents = _settings->rendererSettings.windowExtents;

  context.ClientInitialize();
}

void Piano::Application::Shutdown()
{
  context.ClientShutdown();

  Piano::Renderer::Shutdown();
  Piano::Platform::Shutdown();
}

// ==========
// Update
// ==========

void UpdateTime()
{
  frameEnd = std::chrono::steady_clock::now();

  Piano::time.deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(frameEnd - frameStart).count() * microToSecond;
  Piano::time.totalTime = std::chrono::duration_cast<std::chrono::microseconds>(frameEnd - realtimeStart).count() * microToSecond;

  Piano::time.frameCount++;

  frameStart = frameEnd;
}

void Piano::Application::HandleInput(GLFWwindow* _window)
{
  if (glfwGetKey(_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(_window, true);

  // TODO : Update the status of all applicable keys (keyboard & piano)
}

void Piano::Application::MainLoop()
{
  while (!Piano::Platform::ShouldClose())
  {
    // Frame initializtation =====
    {
      HandleInput(Piano::Platform::GetWindow());
    }

    // Client loop =====
    {
      context.ClientUpdate(Piano::time.deltaTime);
    }

    // Rendering =====
    {
      Piano::Renderer::RenderFrame();

      glfwSwapBuffers(Piano::Platform::GetWindow());
      glfwPollEvents();
    }

    UpdateTime();
  }
}

// ==========
// Functionality
// ==========

void Piano::Application::PlaceNoteOnTimeline(u32 _note, f32 _startTime, f32 _duration)
{
  // Calculate note's world position =====
  f32 worldX = ((_note * 80.0f) - context.windowExtents.width * 0.45f) / (context.windowExtents.width * 0.5f);
  f32 worldY = _startTime;

  // Place and scale the note =====
  PianoLogWarning("Need to write note placement code");

  //quadIndex = (quadIndex + 1) % quadCount;
}

UiElement Piano::Application::AddUiElement(vec2 _position, vec2 _scale /*= {1, 1}*/)
{
  // Calculate clip-space position

  return {};
}
