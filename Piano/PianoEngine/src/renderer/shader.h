#pragma once

#include "defines.h"

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

