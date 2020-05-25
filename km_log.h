#pragma once

#include "km_defines.h"
#include "km_lib.h"
#include "km_string.h"

#define LOG_BUFFER_SIZE KILOBYTES(128)
#define LOG_EVENTS_MAX ((LOG_BUFFER_SIZE) / 32)

#define FUNCTION_NAME_MAX_LENGTH 128

enum class LogCategory
{
    ERROR = 0, // Starts at 0 because it is an index into LOG_CATEGORY_NAMES
    WARNING,
    INFO,
    DEBUG
};

const char* LOG_CATEGORY_NAMES[] = {
    "ERROR",
    "WARN",
    "INFO",
    "DEBUG"
};

struct LogEvent
{
    LogCategory category;
    char file[PATH_MAX_LENGTH];
    int line;
    char function[FUNCTION_NAME_MAX_LENGTH];
    uint64 logStart;
    uint64 logSize;
};

struct LogState
{
    uint64 eventFirst, eventCount;
    LogEvent logEvents[LOG_EVENTS_MAX];
    char buffer[LOG_BUFFER_SIZE];
    
    void PrintFormat(LogCategory logCategory,
                     const char* file, int line, const char* function,
                     const char* format, ...);
};

global_var LogState* logState_;

void PlatformFlushLogs(LogState* logState);

#define LOG_ERROR(format, ...) logState_->PrintFormat(LogCategory::ERROR, \
__FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define LOG_WARN(format, ...) logState_->PrintFormat(LogCategory::WARNING, \
__FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) logState_->PrintFormat(LogCategory::INFO, \
__FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define LOG_FLUSH() PlatformFlushLogs(logState_)
#if GAME_SLOW
#define LOG_DEBUG(format, ...) logState_->PrintFormat(LogCategory::DEBUG, \
__FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#else
#define LOG_DEBUG(format, ...)
#endif