#pragma once

#include <cstddef>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

// STATIC REDEFINES
#define internal      static
#define local_persist static
#define global_var    static

/*
GAME_INTERNAL:
0 - build for public release
1 - build for developer only

GAME_SLOW:
0 - slow code NOT allowed
1 - slow code allowed (asserts, etc)

GAME_WIN32:
defined if compiling for Win32 platform
*/

#define ARRAY_COUNT(arrayName) (sizeof(arrayName) / sizeof((arrayName)[0]))

// Added (val - val) here to force integral promotion to the size of Value
#define ALIGN_POW2(val, alignment) \
((val + ((alignment) - 1)) & ~((val - val) + (alignment) - 1))
#define ALIGN4(val) ((val + 3) & ~3)
#define ALIGN8(val) ((val + 7) & ~7)
#define ALIGN16(val) ((val + 15) & ~15)

#define KILOBYTES(bytes) ((bytes) * 1024LL)
#define MEGABYTES(bytes) (KILOBYTES(bytes) * 1024LL)
#define GIGABYTES(bytes) (MEGABYTES(bytes) * 1024LL)
#define TERABYTES(bytes) (GIGABYTES(bytes) * 1024LL)

// NUMERIC TYPES
using int8    = int8_t;
using int16   = int16_t;
using int32   = int32_t;
using int64   = int64_t;

using uint8   = uint8_t;
using uint16  = uint16_t;
using uint32  = uint32_t;
using uint64  = uint64_t;

using float32 = float;
using float64 = double;

constexpr int16  INT16_MINVAL = -32768;
constexpr int16  INT16_MAXVAL = 32767;
constexpr uint16 UINT16_MAXVAL = 65535;
