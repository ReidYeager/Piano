
#include "defines.h"

#include "platform/platform.h"
#include "math/vector.h"

#include <GLFW/glfw3.h>
#include <stdio.h>

// TODO : ~!!~ Logger

Platform platform;

GLFWwindow* Platform::Initialize(vec2 _windowExtents)
{
  // Initialize GLFW
  window = InitializeWindow(_windowExtents.width, _windowExtents.height);
  return window;
}

void Platform::Shutdown()
{
  glfwDestroyWindow(window);
}

b8 Platform::ShouldClose()
{
  return glfwWindowShouldClose(window);
}

GLFWwindow* Platform::InitializeWindow(u32 _width, u32 _height)
{
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  GLFWwindow* w = glfwCreateWindow(_width, _height, "PianoHero", nullptr, nullptr);
  if (w == nullptr)
  {
    printf("Failed to opend glfw window\n");
    glfwTerminate();
    throw(-1);
  }

  glfwMakeContextCurrent(w);

  return w;
}