#pragma once

#include <stdlib.h>

#include "km_defines.h"

#if GAME_SLOW

// TODO review this log dependency
#include "km_log.h"

#define DEBUG_ASSERTF(expression, format, ...) if (!(expression)) { \
    LOG_ERROR("Assert failed:\n"); \
    LOG_ERROR(format, ##__VA_ARGS__); \
    PlatformFlushLogs(logState_); \
    abort(); }
#define DEBUG_ASSERT(expression) DEBUG_ASSERTF(expression, "")
#define DEBUG_PANIC(format, ...) \
	LOG_ERROR("PANIC!\n"); \
	LOG_ERROR(format, ##__VA_ARGS__); \
	PlatformFlushLogs(logState_); \
    abort();

#elif GAME_INTERNAL

#include "km_log.h"

#define DEBUG_ASSERTF(expression, format, ...) if (!(expression)) { \
    LOG_ERROR("Assert failed\n"); \
    LOG_ERROR(format, ##__VA_ARGS__); \
    PlatformFlushLogs(logState_); }
#define DEBUG_ASSERT(expression) DEBUG_ASSERTF(expression, "")
#define DEBUG_PANIC(format, ...) \
    LOG_ERROR("PANIC!\n"); \
    LOG_ERROR(format, ##__VA_ARGS__); \
    PlatformFlushLogs(logState_);

#else
// TODO rethink these macros maybe, at least the panic
#define DEBUG_ASSERTF(expression, format, ...)
#define DEBUG_ASSERT(expression)
#define DEBUG_PANIC(format, ...)
#endif
