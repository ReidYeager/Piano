
#include "defines.h"
#include "logger.h"

#include "platform/platform.h"
#include "math/vector.h"

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <vector>

std::thread threads[64];
u64 availableThreads = -2; // Thread 0 is invalid thread & will always be set to 0

//=========================
// Memory
//=========================

void* Piano::MemoryAllocate(u64 _size)
{
  return malloc(_size);
}

void Piano::MemorySet(void* _data, u64 _size, u32 _value)
{
  memset(_data, _value, _size);
}

void Piano::MemoryCopy(void* _source, void* _destination, u64 _size)
{
  memcpy(_destination, _source, _size);
}

void Piano::MemoryFree(void* _data)
{
  free(_data);
}

//=========================
// Platform
//=========================

GLFWwindow* window = nullptr;

b8 InitializeWindow(u32 _width, u32 _height)
{
  if (!glfwInit())
  {
    PianoLogFatal("Failed to init GLFW %d", 1);
    return false;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
  //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
  //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  window = glfwCreateWindow(_width, _height, "PianoHero", nullptr, nullptr);
  if (window == nullptr)
  {
    PianoLogFatal("Failed to open glfw window %d", 1);
    glfwTerminate();
    return false;
  }

  glfwMakeContextCurrent(window);

  return true;
}

b8 Piano::Platform::Initialize(vec2I _windowExtents)
{
  return InitializeWindow(_windowExtents.width, _windowExtents.height);
}

b8 Piano::Platform::Shutdown()
{
  glfwDestroyWindow(window);

  for (u32 i = 1; i < 64; i++)
  {
    if (!(availableThreads & (u64(1) << i)))
    {
      threads[i].join();
    }
  }

  return true;
}

b8 Piano::Platform::ShouldClose()
{
  return glfwWindowShouldClose(window);
}

GLFWwindow* Piano::Platform::GetWindow()
{
  return window;
}

#if PIANO_PLATFORM_LINUX
void Piano::Platform::ExecuteCommand(const char* _command)
{
  system(_command);
}

#elif PIANO_PLATFORM_WINDOWS
void Piano::Platform::ExecuteCommand(const char* _command)
{
  PianoLogWarning("ExecuteCommand is only run on Linux.\n> Did not execute '%s'", _command);
}
#endif

u32 Piano::Platform::StartThread(void(*_function)())
{
  if (!availableThreads) // Ensure at least one thread is unused
  {
    PianoLogError("All %d threads are in use.", 63);
    return 0;
  }

  u64 firstAvailableIndex = 0;
  u64 lowestBit = availableThreads & -availableThreads;
  firstAvailableIndex += ((lowestBit & 0xffffffff00000000) != 0) * 32;
  firstAvailableIndex += ((lowestBit & 0xffff0000ffff0000) != 0) * 16;
  firstAvailableIndex += ((lowestBit & 0xff00ff00ff00ff00) != 0) * 8;
  firstAvailableIndex += ((lowestBit & 0xf0f0f0f0f0f0f0f0) != 0) * 4;
  firstAvailableIndex += ((lowestBit & 0xcccccccccccccccc) != 0) * 2;
  firstAvailableIndex += ((lowestBit & 0xaaaaaaaaaaaaaaaa) != 0) * 1;

  availableThreads &= ~(u64(1) << firstAvailableIndex);
  threads[firstAvailableIndex] = std::thread(_function);

  return (u32)firstAvailableIndex;
}

void Piano::Platform::EndThread(u32 _thread)
{
  if (!_thread) // If 0, ignore.
    return;

  threads[_thread].join();
  availableThreads |= (u64(1) << _thread);
}

#if PIANO_PLATFORM_LINUX
void Piano::Platform::PrintToConsole(const char* _message, u32 _color)
{
  //                               INFO,  DEBUG,   WARN,  ERROR,  FATAL
  const char* colorStrings[] = { "1;37", "1;34", "1;33", "1;31", "1;41" };
  printf("\033[%sm%s\033[0m", colorStrings[_color], _message);
}
#elif PIANO_PLATFORM_WINDOWS
#include <windows.h>

void Piano::Platform::PrintToConsole(const char* _message, u32 _color)
{
  //               Info , Debug, Warning, Error , Fatal
  //               White, Cyan , Yellow , Red   , White-on-Red
  u32 colors[] = { 0xf  , 0xb  , 0xe    , 0x4   , 0xcf };

  HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
  SetConsoleTextAttribute(console, colors[_color]);
  OutputDebugStringA(_message);
  u64 length = strlen(_message);
  LPDWORD written = 0;
  WriteConsoleA(console, _message, (DWORD)length, written, 0);
  SetConsoleTextAttribute(console, 0xf);
}
#endif
