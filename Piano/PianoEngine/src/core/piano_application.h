
#ifndef PIANO_PIANO_APPLICATION_H_
#define PIANO_PIANO_APPLICATION_H_

#include "defines.h"

#include "math/vector.h"
#include "platform/platform.h"
#include "renderer/renderer.h"
#include "ui/ui_element.h"

#include <GLFW/glfw3.h>

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
    void(*Initialize)();
    void(*Update)(float);
    void(*Shutdown)();

    Piano::Renderer::RendererSettings rendererSettings;
  };

  // Defines API function calls
  class Application
  {
  private:
    struct
    {
      void(*ClientInitialize)();
      void(*ClientUpdate)(float);
      void(*ClientShutdown)();

      vec2 windowExtents;
    } context;

  private:
    // Initializes the engine's components
    void Initialize(Piano::ApplicationSettings* _settings);
    // Handles everything that occurs every frame
    void MainLoop();
    // Destroys the engine's components
    void Shutdown();
    // Updates the input state
    void HandleInput(GLFWwindow* _window);

  public:
    void Run(ApplicationSettings* _settings);

    void PlaceNoteOnTimeline(u32 _note, f32 _startTime, f32 _duration);
    UiElement AddUiElement(vec2 _position, vec2 _scale = {1, 1});

  };

}

#endif // !PIANO_H_
