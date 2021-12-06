
#ifndef PIANO_PIANO_APPLICATION_H_
#define PIANO_PIANO_APPLICATION_H_

#include "defines.h"

#include "math/vector.h"
#include "platform/platform.h"
#include "renderer/renderer.h"
#include "ui/ui_element.h"

#include <GLFW/glfw3.h>

struct phApplicationSettings
{
  void(*Initialize)();
  void(*Update)(double);
  void(*Shutdown)();
};

// Defines API function calls
class PianoApplication
{
private:
  struct
  {
    void(*ClientInitialize)();
    void(*ClientUpdate)(double);
    void(*ClientShutdown)();
  } context;

private:
  // Initializes the engine's components
  void Initialize(phApplicationSettings* _settings);
  // Handles everything that occurs every frame
  void MainLoop();
  // Destroys the engine's components
  void Shutdown();

  void HandleInput(GLFWwindow* _window);

public:
  void Run(phApplicationSettings* _settings);

  void PlaceNoteOnTimeline(u32 _note, float _startTime, float _duration);
  UiElement AddUiElement(vec2 _position, vec2 _scale = {1, 1});

};

#endif // !PIANO_H_
