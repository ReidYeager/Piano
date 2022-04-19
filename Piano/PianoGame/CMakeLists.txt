# CMakeList.txt : CMake project for PianoHero, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)



# Add source to this project's executable.
add_executable (PianoGame
 "src/source.cpp"
  "src/midi.cpp")

find_package(OpenGL REQUIRED)

target_include_directories(PianoGame PRIVATE
 ./src
 ${CMAKE_SOURCE_DIR}/PianoEngine/src
 ${CMAKE_SOURCE_DIR}/libraries/include
)

target_link_directories (PianoGame PRIVATE
 ./src
 ${CMAKE_SOURCE_DIR}/PianoEngine/src
 ${CMAKE_SOURCE_DIR}/libraries/lib
)

target_link_libraries(PianoGame
 glfw
 OpenGL::GL
 PianoEngine
)

# TODO: Add tests and install targets if needed.
