# CMakeList.txt : CMake project for PianoHero, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (PianoHero
 "src/source.cpp"
 "src/glad.c"
 "src/defines.h"
 "src/ui/ui_element.h"
 "src/renderer/renderer.cpp"
 "src/renderer/renderer.h"
 "src/renderer/shader.h"
 "src/platform/platform.h"
 "src/platform/platform.cpp"
 "src/platform/filesystem.h"
 "src/math/vector.h"
 "src/core/piano.cpp"
 "src/core/piano.h"
)

target_include_directories(PianoHero PRIVATE
 ./src
 ../libraries/include
)

target_link_directories (PianoHero PRIVATE
 ./src
 ../libraries/lib
)

target_link_libraries(PianoHero
 glfw3.lib
)

# TODO: Add tests and install targets if needed.
