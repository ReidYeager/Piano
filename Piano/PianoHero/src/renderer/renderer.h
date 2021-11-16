
#ifndef PIANO_RENDERER_RENDERER_H_
#define PIANO_RENDERER_RENDERER_H_

#include "defines.h"

#include "renderer/shader.h"
#include "math/vector.h"
#include "platform/platform.h"
#include "ui/ui_element.h"

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

#include <vector>

class Renderer
{
private:
  Platform* platform;

private:
  u32 LoadShader(const char* _filename, GLenum _stage);
  material_t CreateShaderProgram(std::vector<const char*> _filenames, std::vector<GLenum> _stages);
  u32 GetShaderProgram(std::vector<const char*> _filenames, std::vector<GLenum> _stages);

public:
  void Initialize(Platform* _platform);
  void RenderFrame(glm::mat4 mvp);
  void Shutdown();

  // Adds a note to the timeline scene
  // Param : _note = The location of the note on the timeline's X axis
  // Param : _startTime = The location of the start of the note on the timeline's time axis (y)
  // Param : _duration = The scale of the note on the timeline's time axis (y)
  void AddNoteToTimeline(u32 _note, float _startTime, float _duration);
  // Add color? Effects?

  // Plays a particle effect at a location on screen
  // Does nothing until a particle system is implemented
  // Param : _effect = The effect to be played (Placeholder, for future use)
  // Param : _position = The screen position in pixel coordinates to play to effect
  void PlayEffect(vec2 _position, void* _effect);

  // Abstracts away the exact positioning for playing an effect above a key
  // Param : _effect = The effect to be played (Placeholder, for future use)
  // Param : _note = The note to play the effect above
  void PlayEffect(void* _effect, u32 _note);

  // Adds an element onto the screen
  // Param : _position = The location of the element's top-left corner in pixel coordinates
  // Param : _extents = The width (x) and height (y) of the element in pixels
  UiElement CreateUiElement(vec2 _position, vec2 _extents);


};

#endif //!PIANO_RENDERER_RENDERER_H_
