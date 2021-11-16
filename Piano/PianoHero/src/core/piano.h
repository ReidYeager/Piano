
#ifndef PIANO_H_
#define PIANO_H_

#include "defines.h"

#include "math/vector.h"
#include "platform/platform.h"
#include "renderer/renderer.h"
#include "ui/ui_element.h"

#include <GLFW/glfw3.h>

// Defines API function calls
class PianoApplication
{
private:
  Platform platform;
  Renderer renderer;

private:
  void Initialize();
  void Loop();
  void Shutdown();

  void HandleInput(GLFWwindow* _window);

public:
  void Run();

  void PlaceNoteOnTimeline(u32 _note, float _startTime, float _duration);
  UiElement AddUiElement(vec2 _position, vec2 _scale = {1, 1});

};

#endif // !PIANO_H_
