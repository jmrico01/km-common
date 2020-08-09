#pragma once

#include "km_defines.h"

#if GAME_SLOW

#define DEBUG_PANIC(format, ...) \
LOG_ERROR("PANIC! %s:%d (%s)\n", __FILE__, __LINE__, __func__); \
LOG_ERROR(format, ##__VA_ARGS__); \
LOG_FLUSH(); \
*(int*)0 = 0

#define DEBUG_ASSERTF(expression, format, ...) if (!(expression)) { \
LOG_ERROR("Assertion failed at %s:%d (%s)\n", __FILE__, __LINE__, __func__); \
DEBUG_PANIC(format, ##__VA_ARGS__); }
#define DEBUG_ASSERT(expression) DEBUG_ASSERTF(expression, "")

#else

#define DEBUG_PANIC(format, ...)
#define DEBUG_ASSERTF(expression, format, ...)
#define DEBUG_ASSERT(expression)

#endif
