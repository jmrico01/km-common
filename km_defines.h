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

// https://stackoverflow.com/questions/4415524/common-array-length-macro-for-c
#define C_ARRAY_LENGTH(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

// defer C++11 implementation, source: https://www.gingerbill.org/article/2015/08/19/defer-in-cpp/
template <typename F>
struct _DeferFunctionObject {
    F function;
    _DeferFunctionObject(F function) : function(function) {}
    ~_DeferFunctionObject() { function(); }
};

template <typename F>
_DeferFunctionObject<F> _DeferFunction(F function) {
    return _DeferFunctionObject<F>(function);
}

#define KM_CONCAT_(x, y) x##y
#define KM_CONCAT(x, y)  KM_CONCAT_(x, y)

#define KM_UNIQUE_NAME_COUNTER(x) KM_CONCAT(x, __COUNTER__)
#define KM_UNIQUE_NAME_LINE(x)    KM_CONCAT(x, __LINE__)

#define defer(code) auto KM_UNIQUE_NAME_COUNTER(_defer_) = _DeferFunction([&](){code;})

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

const int8  INT8_MIN_VALUE  = -128;
const int8  INT8_MAX_VALUE  = 127;
const uint8 UINT8_MAX_VALUE = 0xff;

const int16  INT16_MIN_VALUE  = -32768;
const int16  INT16_MAX_VALUE  = 32767;
const uint16 UINT16_MAX_VALUE = 0xffff;

const uint32 UINT32_MAX_VALUE = 0xffffffff;

const uint64 UINT64_MAX_VALUE = 0xffffffffffffffff;
