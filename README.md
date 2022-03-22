# Piano Heroes
---
## Introduction

Piano hero is the senior design project of Mohammad Abdellatif, Myles Guiam, Ryan Tucholski, and Reid Yeager overseen by Shawn Gieser.

## Compilation
Piano hero uses GLFW as a windowing library and includes it as a submodule. Include `--recursive` when cloning the project.
To compile on a linux platform, GLFW's dependencies must me available as well. You can find instructions on GLFW compilation [here](https://www.glfw.org/docs/latest/compile.html)

Compilation uses CMake to allow for simple compilation across platforms. Use either the CMake GUI or CMake command line interface.

In the `Piano` directory containting `PianoEngine` and `PianoGame` subdirectories:
> cmake -S . -B ./{pathToBuild}

This will create a new subdirectory for the project's built files.
In the {pathToBuild} subdirectory:
> make

The program can then be executed using the executable file `PianoGame` located in {pathToBuild}/PianoGame/