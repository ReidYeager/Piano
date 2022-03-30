
#ifndef PIANO_RENDERER_RENDERER_H_
#define PIANO_RENDERER_RENDERER_H_

#include "defines.h"

#include "renderer/renderer_context.h"

#include "math/vector.h"

#include <glm/glm.hpp>

#include <string>
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

  struct TextGlyph
  {
    u32 textureID;  // ID handle of the glyph texture
    glm::ivec2   size;       // Size of glyph
    glm::ivec2   bearing;    // Offset from baseline to left/top of glyph
    u32 advance;    // Offset to advance to next glyph
  };

  namespace Renderer {
    struct RendererSettings
    {
      vec2I windowExtents; // Pixel width and height of the window
      f32 keyboardViewWidth;   // Used to determine the required projection width
      f32 previewDuration; // Number of seconds to preview
    };

    // Initializes OpenGL and creates a surface and swapchain
    b8 Initialize(Piano::Renderer::RendererSettings _settings);
    b8 RenderFrame();
    b8 Shutdown();

    // Functionality =====
    // Changes the camera's projection of the roll
    void ChangeCameraSettings(float _width, f32 _previewDuration);
    // Places the camera at the given time on the roll
    void PlaceCamera(f32 _time);
    // Define the notes to be rendered
    void SetNotes(const std::vector<Piano::note>& _notes);
    void AddText(const std::string& _text,
                 vec2 _startPosition,
                 vec3 _color = {1.0f, 1.0f, 1.0f},
                 f32 _scale = 1.0f,
                 b8 _printToWorldspace = false);
    void ClearText(b8 _clearWorldText);
} }

#endif //!PIANO_RENDERER_RENDERER_H_
