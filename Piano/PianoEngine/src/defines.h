
#ifndef PIANO_DEFINES_H_
#define PIANO_DEFINES_H_

#ifndef GLADINCLUDED
#define GLADINCLUDED
#include <glad/glad.h>
#endif // !GLADINCLUDED

#define PIANO_DEBUG 1 // Change to 0 to not use debug components

//=========================
// Platform detection
//=========================

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define PIANO_PLATFORM_WINDOWS 1
#elif defined(__linux__) || defined(__gnu_linux__)
#define PIANO_PLATFORM_LINUX 1
#else
#error "This platform is currently not supported"
#endif // Platform detection

//=========================
// Types
//=========================

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

#endif // !PIANO_DEFINES_H_
