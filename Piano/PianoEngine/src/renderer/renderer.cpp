
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
glm::mat4 projectionMatrix;
glm::mat4 viewProjectionMatrix = glm::mat4(1.0f);

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
  int size = code.size();

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

void CreateShader(const char* _vert, const char* _frag)
{
  noteMaterial = {};
  u32 vs, fs;

  // Create shader stages =====
  vs = LoadShader(_vert, GL_VERTEX_SHADER);
  fs = LoadShader(_frag, GL_FRAGMENT_SHADER);

  noteMaterial.vertFile = _vert;
  noteMaterial.fragFile = _frag;

  // Setup shader program =====
  noteMaterial.shaderProgram = glCreateProgram();
  glAttachShader(noteMaterial.shaderProgram, vs);
  glAttachShader(noteMaterial.shaderProgram, fs);
  glLinkProgram(noteMaterial.shaderProgram);

  int success;
  char infoLog[512];
  glGetProgramiv(noteMaterial.shaderProgram, GL_LINK_STATUS, &success);
  if (!success)
  {
    glGetProgramInfoLog(noteMaterial.shaderProgram, 512, NULL, infoLog);
    PianoLogFatal("Program linking failed\n%s\n", infoLog);
    throw(-1);
  }

  glDeleteShader(vs);
  glDeleteShader(fs);

  // Bind vertex info =====
  glGenBuffers(1, &noteMaterial.vbo);
  glGenBuffers(1, &noteMaterial.ebo);
  glGenVertexArrays(1, &noteMaterial.vao);

  glBindVertexArray(noteMaterial.vao);

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

  glBindBuffer(GL_ARRAY_BUFFER, noteMaterial.vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, noteMaterial.ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
}

void Piano::Renderer::Initialize(Piano::Renderer::RendererSettings _settings)
{
  // Initialize GLAD / OpenGL =====
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    printf("Failed to initialize GLAD\n");
    throw(-1);
  }

  // Create swapchain
  glViewport(0, 0, _settings.windowExtents.x, _settings.windowExtents.y);

  RecalibrateCamera(_settings.camera);
  PlaceCamera(0.0f);

  // Create the note shader
  CreateShader("base.vert", "base.frag");
}

void Piano::Renderer::Shutdown()
{
  
}

//=========================
// Update
//=========================

void Piano::Renderer::RenderFrame()
{
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(noteMaterial.shaderProgram);
  u32 noteTransformID = glGetUniformLocation(noteMaterial.shaderProgram, "worldspaceTransform");
  u32 viewID = glGetUniformLocation(noteMaterial.shaderProgram, "viewMatrix");

  glUniformMatrix4fv(noteTransformID, 1, GL_FALSE, glm::value_ptr(identityMatrix));
  glUniformMatrix4fv(viewID, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));

  // For each note =====
  {
    // Use the note's transform
    //glUniformMatrix4fv(matid, 1, GL_FALSE, glm::value_ptr(transform));

    // Render one note quad =====
    glBindVertexArray(noteMaterial.vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
  }
}

//=========================
// Functionality
//=========================

void Piano::Renderer::RecalibrateCamera(CameraSettings _settings)
{
  // TODO : Properly calculate the camera width calibration value (currently using random values)

  f32 right = 0.0f;
  switch (_settings.keyboardLayout)
  {
  case Keyboard_Full: right = 2.0f; break;
  case Keyboard_Half: right = 1.0f; break;
  }

  projectionMatrix = glm::ortho(0.0f, right, 0.0f, _settings.previewLength);
}

void Piano::Renderer::PlaceCamera(f32 _time)
{
  glm::mat4 view = glm::translate(identityMatrix, glm::vec3(0.0f, _time, -0.5f));
  viewProjectionMatrix = view * projectionMatrix;
}
