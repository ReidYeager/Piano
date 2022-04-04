# CMakeList.txt : CMake project for PianoHero, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

#set(GLFW_BUILD_DOCS OFF CAHCE BOOL "" FORCE)
#set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
#set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(glfw_repo)
add_subdirectory(freetype-2.11.1)

find_package(OpenGL REQUIRED)

# Add source to this project's executable.
add_library (PianoEngine STATIC
 "src/platform/glad.c"
 "src/defines.h"
 "src/ui/ui_element.h"
 "src/renderer/renderer.cpp"
 "src/renderer/renderer.h"
 "src/platform/platform.h"
 "src/platform/platform.cpp"
 "src/platform/filesystem.h"
 "src/math/vector.h"
 "src/core/piano_application.cpp"
 "src/core/piano_application.h"
 "src/piano_hero.h" "src/logger.h" "src/renderer/renderer_context.h")

target_include_directories(PianoEngine PRIVATE
 ./src
 ${CMAKE_SOURCE_DIR}/libraries/include
 ${CMAKE_SOURCE_DIR}/PianoEngine/freetype-2.11.1/include
)

target_link_directories (PianoEngine PRIVATE
 ./src
 ${CMAKE_SOURCE_DIR}/libraries/lib
)

target_link_libraries(PianoEngine
 glfw
 freetype
 OpenGL::GL
)

#add_compile_definitions(PIANO_RESOURCE_DIR="${CMAKE_SOURCE_DIR}/PianoGame/res/")

