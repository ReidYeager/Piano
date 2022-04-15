
#ifndef PIANO_RENDERER_RENDERER_CONTEXT_H_
#define PIANO_RENDERER_RENDERER_CONTEXT_H_

#include "defines.h"

namespace Piano {
  #define BLACK_KEY_WIDTH 0.5f
  #define WHITE_KEY_WIDTH 0.75f
  typedef vec4 note;

  namespace Renderer {
    enum KeyboardLayoutTypes
    {
      // TODO : Actually define keyboard layouts
      Keyboard_Full,
      Keyboard_Half
    };
} }

#endif // !PIANO_RENDERER_RENDERER_CONTEXT_H_
