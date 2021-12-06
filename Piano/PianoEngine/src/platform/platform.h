
#ifndef PIANO_PLATFORM_PLATFORM_H_
#define PIANO_PLATFORM_PLATFORM_H_

#include "defines.h"

#include "GLFW/glfw3.h"

extern class Platform
{
private:
  GLFWwindow* window;

public:
  // Initializes platform-dependent components
  // Param : _width = The window width in pixels
  // Param : _height = The window height in pixels
  GLFWwindow* Initialize(u32 _width, u32 _height);
  // Destroys platform-dependent components
  void Shutdown();

  b8 ShouldClose();

  GLFWwindow* GetWindow() { return window; }

private:
  GLFWwindow* InitializeWindow(u32 _width, u32 _height);

} pianoPlatform;

#endif // !PIANO_PLATFORM_PLATFORM_H_
