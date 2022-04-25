
#ifndef PIANO_PIANO_APPLICATION_H_
#define PIANO_PIANO_APPLICATION_H_

#include "defines.h"
#include "logger.h"

#include "math/vector.h"
#include "platform/platform.h"
#include "renderer/renderer.h"
#include "ui/ui_element.h"

#include <GLFW/glfw3.h>

#include <stdarg.h>

namespace Piano {

  //=========================
  // Time
  //=========================

  extern struct TimeInfo
  {
    f32 totalTime; // Real-time in seconds since the application started

    f32 deltaTime; // Time in seconds taken by the previous tick

    u32 frameCount; // Number of frames rendered before this tick
  } time;

  //=========================
  // Application
  //=========================

  struct ApplicationSettings
  {
    b8(*InitFunction)();
    b8(*UpdateFunction)(float);
    b8(*ShutdownFunction)();

    Piano::Renderer::RendererSettings rendererSettings;
  };

  struct TextPrintSettings
  {
    vec2 startPosition = { 0.0f, 0.0f };
    f32 scale = 1.0f;
    vec3 color = { 1.0f, 1.0f, 1.0f };
  };

  // Defines API function calls
  class Application
  {
  private:
    struct
    {
      b8(*ClientInitialize)();
      b8(*ClientUpdate)(float);
      b8(*ClientShutdown)();
    } context;

  private:
    // Initializes the engine's components
    b8 Initialize(Piano::ApplicationSettings* _settings);
    // Handles everything that occurs every frame
    b8 MainLoop();
    // Destroys the engine's components
    b8 Shutdown();
    // Updates the input state
    void HandleInput(GLFWwindow* _window);

  public:
    void Run(ApplicationSettings* _settings);

    void ExecuteCommand(const char* _command);

    void PlaceNoteOnTimeline(u32 _note, f32 _startTime, f32 _duration);
    void PushNotesTimelineToRenderer();
    void ClearNotesTimeline();
    void SetRenderOctaveLines(b8 _value);
    void PrintToScreen(TextPrintSettings _settings, const char* _text, ...);
    void PrintToWorld(TextPrintSettings _settings, const char* _text, ...);
    void ClearScreenText() { Piano::Renderer::ClearText(false); }
    void ClearWorldText() { Piano::Renderer::ClearText(true); }
  };

}

#endif // !PIANO_H_
