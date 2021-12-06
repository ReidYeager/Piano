
#ifndef PIANO_CORE_GLOBAL_STATS_H_
#define PIANO_CORE_GLOBAL_STATS_H_

#include "defines.h"

#include <math/vector.h>

static struct
{
  f32 realTime; // Real time in seconds since the main loop began
  f32 deltaTime; // Real time in milliseconds of the previous frame
} globalStats;

#endif // !define PIANO_CORE_GLOBAL_STATS_H_
