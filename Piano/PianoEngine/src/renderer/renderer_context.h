
#ifndef PIANO_RENDERER_RENDERER_CONTEXT_H_
#define PIANO_RENDERER_RENDERER_CONTEXT_H_

#include "defines.h"

namespace Piano {
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
