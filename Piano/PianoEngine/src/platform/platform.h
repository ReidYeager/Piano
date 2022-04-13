
#ifndef PIANO_PLATFORM_PLATFORM_H_
#define PIANO_PLATFORM_PLATFORM_H_

#include "defines.h"

#include "math/vector.h"

#include "GLFW/glfw3.h"

namespace Piano {
  void* MemoryAllocate(u64 _size);
  void MemorySet(void* _data, u64 _size, u32 _value);
  void MemoryCopy(void* _source, void* _destination, u64 _size);
  void MemoryFree(void* _data);

  inline void MemoryZero(void* _data, u64 _size) { Piano::MemorySet(_data, _size, 0); }

namespace Platform {
  // Prints a string to the device's console output
  void PrintToConsole(const char* _message, u32 _color);
  // Initializes platform-dependent components and creates a window
  b8 Initialize(vec2I _windowExtents);
  // Destroys platform-dependent components
  b8 Shutdown();
  // Returns true if the window is closing
  b8 ShouldClose();
  // WIndow getter
  GLFWwindow* GetWindow();
  // Executes the command in the platform's terminal
  void ExecuteCommand(const char* _command);

  u32 StartThread(void(*_function)());
  void EndThread(u32 _thread);

} }

#endif // !PIANO_PLATFORM_PLATFORM_H_
