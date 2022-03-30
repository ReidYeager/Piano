
#include "defines.h"
#include "logger.h"

#include "renderer/renderer.h"
#include "platform/filesystem.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <vector>

const glm::mat4 identityMatrix = glm::mat4(1.0f);

Piano::Material noteMaterial;
Piano::Material textMaterial;
glm::mat4 projectionMatrix;
glm::mat4 viewProjectionMatrix = glm::mat4(1.0f);

std::vector<Piano::note> noteTransforms;

//=========================
// Init & Shutdown
//=========================

u32 LoadShader(const char* _filename, GLenum _stage)
{
  std::string dir = PIANO_RESOURCE_DIR;
  dir.append("shaders/");
  dir.append(_filename);

  std::vector<char> code = LoadFile(dir.c_str());
  if (code.size() == 0)
  {
    printf("Failed to open the shader file %s\n", dir.c_str());
    return -1;
  }

  char* c = code.data();
  i32 size = (i32)code.size();

  u32 shader = glCreateShader(_stage);
  glShaderSource(shader, 1, reinterpret_cast<GLchar**>(&c), (GLint*)&size);
  glCompileShader(shader);

  int success;
  char infoLog[512];
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    printf("Shader (%s) failed to compile\n%s\n", dir.c_str(), infoLog);
  }

  return shader;
}

b8 CreateMaterial(const char* _vert, const char* _frag, Piano::Material* _material, b8 _dynamic)
{
  u32 vs, fs;

  // Create shader stages =====
  vs = LoadShader(_vert, GL_VERTEX_SHADER);
  fs = LoadShader(_frag, GL_FRAGMENT_SHADER);

  _material->vertFile = _vert;
  _material->fragFile = _frag;

  // Setup shader program =====
  _material->shaderProgram = glCreateProgram();
  glAttachShader(_material->shaderProgram, vs);
  glAttachShader(_material->shaderProgram, fs);
  glLinkProgram(_material->shaderProgram);

  int success;
  char infoLog[512];
  glGetProgramiv(_material->shaderProgram, GL_LINK_STATUS, &success);
  if (!success)
  {
    glGetProgramInfoLog(_material->shaderProgram, 512, NULL, infoLog);
    PianoLogFatal("Program linking failed\n%s\n", infoLog);
    return false;
  }

  glDeleteShader(vs);
  glDeleteShader(fs);

  // Bind vertex info =====
  glGenBuffers(1, &_material->vbo);
  glGenBuffers(1, &_material->ebo);
  glGenVertexArrays(1, &_material->vao);

  glBindVertexArray(_material->vao);

  const vec3 vertices[4] = {
    { 1.0f, 1.0f, 0.0f}, // TR
    { 1.0f, 0.0f, 0.0f}, // BR
    { 0.0f, 0.0f, 0.0f}, // BL
    { 0.0f, 1.0f, 0.0f}  // TL
  };

  const u32 indices[6] = {
      0, 1, 3,
      1, 2, 3
  };

  glBindBuffer(GL_ARRAY_BUFFER, _material->vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, _dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _material->ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, _dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  return true;
}

b8 Piano::Renderer::Initialize(Piano::Renderer::RendererSettings _settings)
{
  // Initialize GLAD / OpenGL =====
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    PianoLogFatal("Failed to initialize GLAD %d", 1);
    return false;
  }

  glViewport(0, 0, _settings.windowExtents.x, _settings.windowExtents.y);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  // Create materials =====
  if (!CreateMaterial("note.vert", "note.frag", &noteMaterial, false))
  {
    PianoLogFatal("Failed to create note material %d", 1);
    return false;
  }

  if (!CreateMaterial("text.vert", "text.frag", &textMaterial, true))
  {
    PianoLogFatal("Failed to create text material %d", 1);
    return false;
  }

  // Initialize camera =====
  ChangeCameraSettings(_settings.keyboardViewWidth, _settings.previewDuration);
  PlaceCamera(0.0f);

  return true;
}

b8 Piano::Renderer::Shutdown()
{
  glDeleteVertexArrays(1, &noteMaterial.vao);
  glDeleteBuffers(1, &noteMaterial.vbo);
  glDeleteBuffers(1, &noteMaterial.ebo);
  glDeleteShader(noteMaterial.shaderProgram);

  glDeleteVertexArrays(1, &textMaterial.vao);
  glDeleteBuffers(1, &textMaterial.vbo);
  glDeleteBuffers(1, &textMaterial.ebo);
  glDeleteShader(textMaterial.shaderProgram);

  return true;
}

//=========================
// Update
//=========================

b8 Piano::Renderer::RenderFrame()
{
  glClear(GL_COLOR_BUFFER_BIT);

  // Render notes =====
  {
    glUseProgram(noteMaterial.shaderProgram);
    u32 noteTransformVectorID = glGetUniformLocation(noteMaterial.shaderProgram, "transformValues");
    u32 viewID = glGetUniformLocation(noteMaterial.shaderProgram, "viewMatrix");

    glUniformMatrix4fv(viewID, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));

    for (const auto& noteValue : noteTransforms)
    {
      glUniform4fv(noteTransformVectorID, 1, (float*)&noteValue);

      // Render one note quad =====
      glBindVertexArray(noteMaterial.vao);
      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
      glBindVertexArray(0);
    }
  }

  // Render text =====
  {
    // TODO : Text rendering
  }

  return true;
}

//=========================
// Functionality
//=========================

void Piano::Renderer::ChangeCameraSettings(float _width, f32 _previewDuration)
{
  projectionMatrix = glm::ortho(0.0f, _width, 0.0f, _previewDuration);
}

void Piano::Renderer::PlaceCamera(f32 _time)
{
  glm::mat4 view = identityMatrix;
  view[3][1] = -_time;
  viewProjectionMatrix = projectionMatrix * view;
}

void Piano::Renderer::SetNotes(const std::vector<Piano::note>& _notes)
{
  noteTransforms.clear();
  noteTransforms.reserve(_notes.size());
  for (auto& n : _notes)
  {
    noteTransforms.push_back(n);
  }
}
