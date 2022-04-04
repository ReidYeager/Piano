# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("Piano")
add_compile_definitions(PIANO_RESOURCE_DIR="${CMAKE_SOURCE_DIR}/PianoGame/res/")
# Include sub-projects.
add_subdirectory ("PianoEngine")
add_subdirectory ("PianoGame")
