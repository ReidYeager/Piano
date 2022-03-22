
#ifndef PIANO_RENDERER_RENDERER_CONTEXT_H_
#define PIANO_RENDERER_RENDERER_CONTEXT_H_

#include "defines.h"

namespace Piano {
  namespace Renderer {
    enum KeyboardLayoutTypes
    {
      // TODO : Actually define keyboard layouts
      Keyboard_Full,
      Keyboard_Half
    };

    struct CameraSettings
    {
      KeyboardLayoutTypes keyboardLayout; // Used to determine the required projection width
      f32 previewLength; // Number of seconds to preview
    };
} }

#endif // !PIANO_RENDERER_RENDERER_CONTEXT_H_
