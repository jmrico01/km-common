#pragma once

#include "../km_array.h"
#include "../km_defines.h"

#define LOG_ERROR(format, ...) logState_->PrintFormat(LOG_CATEGORY_ERROR, \
__FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define LOG_WARN(format, ...) logState_->PrintFormat(LOG_CATEGORY_WARNING, \
__FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) logState_->PrintFormat(LOG_CATEGORY_INFO, \
__FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define LOG_FLUSH() PlatformFlushLogs(logState_)

#if GAME_SLOW
#define LOG_DEBUG(format, ...) logState_->PrintFormat(LOG_CATEGORY_DEBUG, \
__FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#endif

// NOTE can't make an enum out of this, because the compiler is crazy
const int LOG_CATEGORY_ERROR = 0;
const int LOG_CATEGORY_WARNING = 1;
const int LOG_CATEGORY_INFO = 2;
const int LOG_CATEGORY_DEBUG = 3;

struct LogEvent
{
    static const uint32 FUNCTION_NAME_MAX = 128;

    int category;
    FixedArray<char, PATH_MAX_LENGTH> file;
    int line;
    FixedArray<char, FUNCTION_NAME_MAX> function;
    uint32 logStart, logSize;
};

struct LogState
{
    static const uint32 LOG_BUFFER_SIZE = KILOBYTES(128);
    static const uint32 LOG_EVENTS_MAX = LOG_BUFFER_SIZE / 32;

    uint32 eventFirst, eventCount;
    StaticArray<LogEvent, LOG_EVENTS_MAX> logEvents;
    StaticArray<char, LOG_BUFFER_SIZE> buffer;

    void PrintFormat(int category, const char* file, int line, const char* function, const char* format, ...);
};

extern LogState* logState_;

void PlatformFlushLogs(LogState* logState);
