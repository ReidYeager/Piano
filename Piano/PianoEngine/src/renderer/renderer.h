
#ifndef PIANO_RENDERER_RENDERER_H_
#define PIANO_RENDERER_RENDERER_H_

#include "defines.h"

#include "renderer/renderer_context.h"

#include "math/vector.h"

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
      vec2 windowExtents;

      Piano::Renderer::CameraSettings camera;
    };

    // Initializes OpenGL and creates a surface and swapchain
    void Initialize(Piano::Renderer::RendererSettings _settings);
    void RenderFrame();
    void Shutdown();

    // Functionality =====
    // Changes the camera's projection of the roll
    void RecalibrateCamera(Piano::Renderer::CameraSettings _settings);
    // Places the camera at the given time on the roll
    void PlaceCamera(f32 _time);
} }

#endif //!PIANO_RENDERER_RENDERER_H_
