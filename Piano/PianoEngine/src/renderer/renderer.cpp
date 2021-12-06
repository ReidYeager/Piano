
#include "defines.h"

#include "renderer/renderer.h"
#include "renderer/shader.h"
#include "platform/filesystem.h"
#include "platform/platform.h"
#include "math/vector.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdio.h>
#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include <math.h>

Renderer renderer;

std::vector<material_t> materials;

float vertices[] = {
  -0.5f, 0.0f, 0.0f, // BL
  -0.5f, 1.0f, 0.0f, // TL
   0.5f, 1.0f, 0.0f, // TR
   0.5f, 0.0f, 0.0f, // BR
};
int indices[] = {
  0, 1, 2,
  2, 3, 0
};


void Renderer::Initialize(vec2 _windowExtents)
{
  // Initialize GLAD / OpenGL
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    printf("Failed to initialize GLAD\n");
    throw(-1);
  }

  glViewport(0, 0, 800, 600);

  GetShaderProgram({ "base.vert", "base.frag" }, { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER });
}

void Renderer::RenderFrame(std::vector<glm::mat4> _mvps)
{
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  material_t& m = materials[0];
  glUseProgram(m.shaderProgram);
  u32 matid = glGetUniformLocation(m.shaderProgram, "mvp");

  for (const auto& transform : _mvps)
  {
    glUniformMatrix4fv(matid, 1, GL_FALSE, glm::value_ptr(transform));
    glBindVertexArray(m.vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  }
}

void Renderer::Shutdown()
{
  // destroy objects
}

u32 Renderer::LoadShader(const char* _filename, GLenum _stage)
{
  std::string dir = PIANO_RESOURCE_SHADER_DIR;
  dir.append(_filename);

  std::vector<char> code = LoadFile(dir.c_str());
  if (code.size() == 0)
  {
    printf("Failed to open the shader file %s\n", dir.c_str());
    return -1;
  }

  char* c = code.data();

  u32 shader = glCreateShader(_stage);
  glShaderSource(shader, 1, reinterpret_cast<GLchar**>(&c), nullptr);
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

material_t Renderer::CreateShaderProgram(std::vector<const char*> _filenames, std::vector<GLenum> _stages)
{
  material_t s;

  u32 vs, fs;

  // Set up shader
  vs = LoadShader(_filenames[0], GL_VERTEX_SHADER);
  fs = LoadShader(_filenames[1], GL_FRAGMENT_SHADER);

  s.vertFile = _filenames[0];
  s.fragFile = _filenames[1];

  s.shaderProgram = glCreateProgram();
  glAttachShader(s.shaderProgram, vs);
  glAttachShader(s.shaderProgram, fs);
  glLinkProgram(s.shaderProgram);

  int success;
  char infoLog[512];
  glGetProgramiv(s.shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(s.shaderProgram, 512, NULL, infoLog);
    printf("Program linking failed\n%s\n", infoLog);
  }

  glDeleteShader(vs);
  glDeleteShader(fs);

  // Bind the vertex array to the shader program
  glGenBuffers(1, &s.vbo);
  glGenBuffers(1, &s.ebo);
  glGenVertexArrays(1, &s.vao);

  glBindVertexArray(s.vao);

  glBindBuffer(GL_ARRAY_BUFFER, s.vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s.ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  return s;
}

u32 Renderer::GetShaderProgram(std::vector<const char*> _filenames, std::vector<GLenum> _stages)
{
  u32 i = 0;
  for (const auto& s : materials)
  {
    if (std::strcmp(s.vertFile, _filenames[0]) == 0 && std::strcmp(s.fragFile, _filenames[1]) == 0)
    {
      return s.shaderProgram;
    }

    i++;
  }

  i = materials.size();
  materials.push_back(CreateShaderProgram(_filenames, _stages));

  return materials[i].shaderProgram;
}
