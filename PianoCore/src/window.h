#pragma once

#include "defines.h"

#include <GLFW/glfw3.h>

#include <stdio.h>

GLFWwindow* InitializeWindow(u32 _width, u32 _height, const char* _title)
{
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  GLFWwindow* w = glfwCreateWindow(800, 600, "Piano", nullptr, nullptr);
  if (w == nullptr)
  {
    printf("Failed to opend glfw window\n");
    glfwTerminate();
    throw(-1);
  }

  glfwMakeContextCurrent(w);

  return w;
}
