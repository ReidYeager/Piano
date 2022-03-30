
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
    if (!Initialize(_settings))
      throw("Engine initialization failed");

    if (!MainLoop())
      throw("Engine main loop failed");

    if (!Shutdown())
      throw("Engine shutdown failed");
  }
  catch (const char* e)
  {
    PianoLogFatal("Piano application failed\n >> %s", e);
    return;
  }

  glfwTerminate();
}

//=========================
// Init & Shutdown
//=========================

b8 Piano::Application::Initialize(ApplicationSettings* _settings)
{
  if (!Piano::Platform::Initialize(_settings->rendererSettings.windowExtents))
  {
    PianoLogFatal("Platform initialization failed %d", 1);
    return false;
  }

  if (!Piano::Renderer::Initialize(_settings->rendererSettings))
  {
    PianoLogFatal("Renderer initialization failed %d", 1);
    return false;
  }

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
  context.ClientUpdate = _settings->UpdateFunction;
  context.ClientShutdown = _settings->ShutdownFunction;

  if (!_settings->InitFunction())
  {
    PianoLogFatal("Game failed to initialize %d", 1);
    return false;
  }

  return true;
}

b8 Piano::Application::Shutdown()
{
  if (!context.ClientShutdown())
  {
    PianoLogFatal("Game failed to shutdown properly %d", 1);
    return false;
  }

  if (!Piano::Renderer::Shutdown())
  {
    PianoLogFatal("Renderer failed to shutdown properly %d", 1);
    return false;
  }

  if (!Piano::Platform::Shutdown())
  {
    PianoLogFatal("Platform failed to shutdown properly %d", 1);
    return false;
  }

  return true;
}

//=========================
// Update
//=========================

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

#include <math.h>
std::vector<Piano::note> notes;

b8 Piano::Application::MainLoop()
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
      if (!Piano::Renderer::RenderFrame())
      {
        PianoLogFatal("Rendering failed %d", 1);
        return false;
      }

      glfwSwapBuffers(Piano::Platform::GetWindow());
      glfwPollEvents();
    }

    UpdateTime();
  }

  return true;
}

//=========================
// Functionality
//=========================

void DetermineKeyXValues(int _key, Piano::note* _note)
{
  // Determine if the key is black (and thus slimmer)
  if (_key % 2)
    _note->keyWidth = 0.5f;
  else
    _note->keyWidth = 1.0f;

  // Do some fancy positioning calculation
  _note->keyPosition = (f32)_key;
}

void Piano::Application::PlaceNoteOnTimeline(u32 _note, f32 _startTime, f32 _duration)
{
  // Place and scale the note =====
  Piano::note newNote;

  DetermineKeyXValues(_note, &newNote);
  newNote.startTime = _startTime;
  newNote.duration = _duration;

  notes.push_back(newNote);
}


void Piano::Application::PushNotesTimelineToRenderer()
{
  Piano::Renderer::SetNotes(notes);
}


void Piano::Application::ClearNotesTimeline()
{
  notes.clear();
  PushNotesTimelineToRenderer();
}

void Piano::Application::PrintToScreen(TextPrintSettings _settings, const char* _text, ...)
{
  // Limit 65,535 characters per message
  const u16 length = 0xFFFF;
  char* outMessage = new char[length];
  Piano::MemoryZero(outMessage, length);

  va_list args;
  va_start(args, _text);
  vsnprintf(outMessage, length, _text, args);
  va_end(args);

  Piano::Renderer::AddText(outMessage,
                           _settings.startPosition,
                           _settings.color,
                           _settings.scale,
                           false);
}

void Piano::Application::PrintToWorld(TextPrintSettings _settings, const char* _text, ...)
{
  // Limit 65,535 characters per message
  const u16 length = 0xFFFF;
  char* outMessage = new char[length];
  Piano::MemoryZero(outMessage, length);

  va_list args;
  va_start(args, _text);
  vsnprintf(outMessage, length, _text, args);
  va_end(args);

  Piano::Renderer::AddText(outMessage,
                           _settings.startPosition,
                           _settings.color,
                           _settings.scale,
                           true);
}

