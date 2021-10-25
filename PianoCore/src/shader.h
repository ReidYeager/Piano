#pragma once

#include "defines.h"
#include "filesystem.h"

#include <vector>

struct material_t
{
  u32 vbo;
  u32 vao;
  u32 ebo;

  const char* vertFile;
  const char* fragFile;
  u32 shaderProgram;
};

static std::vector<material_t> materials;

float vertices[] = {
  -0.5f, -0.5f, 0.0f,
  -0.5f,  0.5f, 0.0f,
   0.5f,  0.5f, 0.0f,
   0.5f, -0.5f, 0.0f,
};
int indices[] = {
  0, 1, 2,
  2, 3, 0
};

u32 LoadShader(const char* _filename, GLenum _stage)
{
  std::string dir = "./res/shaders/";
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

material_t CreateShaderProgram(std::vector<const char*> _filenames, std::vector<GLenum> _stages)
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

u32 GetShaderProgram(std::vector<const char*> _filenames, std::vector<GLenum> _stages)
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
