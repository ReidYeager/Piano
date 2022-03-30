#pragma once

#include "defines.h"

#include <fstream>
#include <vector>

namespace Piano {
  inline std::vector<char> LoadFile(const char* _dir)
  {
    std::ifstream file;
    file.open(_dir, std::ios::ate);

    if (!file)
    {
      printf("Failed to open the file %s\n", _dir);
      return {};
    }

    size_t size = file.tellg();
    file.seekg(0);

    std::vector<char> code(size);
    file.read(code.data(), size);

    file.close();
    return code;
  }
}
