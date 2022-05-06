
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
#include <algorithm>
#include <math.h>

std::vector<Piano::note> notes;

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

void Piano::Application::ExecuteCommand(const char* _command)
{
  Piano::Platform::ExecuteCommand(_command);
}

//=========================
// Init & Shutdown
//=========================

b8 Piano::Application::Initialize(ApplicationSettings* _settings)
{  
  if (!Piano::Platform::Initialize(_settings->rendererSettings.fullscreen, &_settings->rendererSettings.windowExtents))
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
  PianoLogInfo("Was rendering %u notes", notes.size());
  notes.clear();
  
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

void DetermineKeyXValues(u32 _key, Piano::note* _note)
{
  const u32 octaveSize = 12;
  u32 keyShifted = _key - 0x24; // Make the lowest C index 0
  u32 octave = floor(keyShifted / octaveSize);
  u32 keyOctave = keyShifted % octaveSize; // Get the key within the octave

  _note->keyWidth = WHITE_KEY_WIDTH;
  f32 blackWidth = BLACK_KEY_WIDTH;
  f32 halfBlack = blackWidth * 0.5f;

  switch (keyOctave)
  {
  // White keys =====
  case 0:
  {
    _note->keyPosition = 0.0f;
  } break;
  case 2:
  {
    _note->keyPosition = 1.0f;
  } break;
  case 4:
  {
    _note->keyPosition = 2.0f;
  } break;
  case 5:
  {
    _note->keyPosition = 3.0f;
  } break;
  case 7:
  {
    _note->keyPosition = 4.0f;
  } break;
  case 9:
  {
    _note->keyPosition = 5.0f;
  } break;
  case 11:
  {
    _note->keyPosition = 6.0f;
  } break;
   //The lowest C of the next octave is technically part of this one as well.
   //This overlap is ignored here
  //case 12:
  //{
  //  _note->keyPosition = 7.0f;
  //} break;

  // Black keys =====
  case 1:
  {
    _note->keyWidth = blackWidth;
    _note->keyPosition = 1.0f - halfBlack;
  } break;
  case 3:
  {
    _note->keyWidth = blackWidth;
    _note->keyPosition = 2.0f - halfBlack;
  } break;
  case 6:
  {
    _note->keyWidth = blackWidth;
    _note->keyPosition = 4.0f - halfBlack;
  } break;
  case 8:
  {
    _note->keyWidth = blackWidth;
    _note->keyPosition = 5.0f - halfBlack;
  } break;
  case 10:
  {
    _note->keyWidth = blackWidth;
    _note->keyPosition = 6.0f - halfBlack;
  } break;

  default:
  {
    PianoLogError("Unknown key index %u -- %u, %u", keyOctave, _key, octave);
  } break;
  }

  _note->keyPosition += octave * 7.0f;
}

void Piano::Application::PlaceNoteOnTimeline(u32 _note, f32 _startTime, f32 _duration)
{
  PianoLogDebug("Adding note %d to timeline", notes.size());
  
  // Place and scale the note =====
  Piano::note newNote;

  newNote.startTime = _startTime;
  newNote.duration = _duration;
  DetermineKeyXValues(_note, &newNote);

  notes.push_back(newNote);
}


void Piano::Application::PushNotesTimelineToRenderer()
{
  PianoLogDebug("PUSH %d NOTES TO %s", notes.size(), "TIMELINE");
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

void Piano::Application::SetRenderOctaveLines(b8 _value)
{
  Piano::Renderer::SetRenderOctaveNotes(_value);
}

u32 CreateThread(void*(*_function)(void*))
{
  return Piano::Platform::StartThread(_function);
}

void KillThread(u32 _thread)
{
  Piano::Platform::EndThread(_thread);
}

