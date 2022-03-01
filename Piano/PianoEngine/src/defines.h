#pragma once

#ifndef GLADINCLUDED
#define GLADINCLUDED
#include <glad/glad.h>
#endif // !GLADINCLUDED

// Integers
typedef signed char        i8;
typedef short              i16;
typedef int                i32;
typedef long long          i64;
// Unsigned Integers
typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;
typedef u64 IceDeviceSize;
// Floats
typedef float              f32;
typedef double             f64;
// Booleans
typedef unsigned char      b8;
typedef unsigned int       b32;

// TODO : Make these dynamic?
//#define PIANO_RESOURCE_TEXTURE_DIR "../../../PianoGame/res/textures/"
//#define PIANO_RESOURCE_SHADER_DIR  "../../../PianoGame/res/shaders/"
//#define PIANO_RESOURCE_MODEL_DIR   "../../../PianoGame/res/models/"

