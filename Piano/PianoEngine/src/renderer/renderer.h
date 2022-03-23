
#ifndef PIANO_RENDERER_RENDERER_H_
#define PIANO_RENDERER_RENDERER_H_

#include "defines.h"

#include "renderer/renderer_context.h"

#include "math/vector.h"

#include <vector>

namespace Piano {
  struct Material
  {
    u32 vbo;
    u32 vao;
    u32 ebo;

    const char* vertFile;
    const char* fragFile;
    u32 shaderProgram;
  };

  namespace Renderer {
    struct RendererSettings
    {
      vec2I windowExtents;                // Pixel width and height of the window
      KeyboardLayoutTypes keyboardLayout; // Used to determine the required projection width
      f32 previewDuration;                // Number of seconds to preview
    };

    // Initializes OpenGL and creates a surface and swapchain
    b8 Initialize(Piano::Renderer::RendererSettings _settings);
    b8 RenderFrame();
    b8 Shutdown();

    // Functionality =====
    // Changes the camera's projection of the roll
    void ChangeCameraSettings(KeyboardLayoutTypes _layout, f32 _previewDuration);
    // Places the camera at the given time on the roll
    void PlaceCamera(f32 _time);
    // Define the notes to be rendered
    void SetNotes(const std::vector<Piano::note>& _notes);
} }

#endif //!PIANO_RENDERER_RENDERER_H_
