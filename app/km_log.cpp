#include "km_log.h"

#include <stb_sprintf.h>

#include "../km_debug.h"
#include "../km_math.h"

LogState* logState_ = nullptr;

const char* LOG_CATEGORY_NAMES[] = {
    "ERROR",
    "WARN",
    "INFO",
    "DEBUG"
};

void LogState::PrintFormat(int category, const char* file, int line, const char* function, const char* format, ...)
{
    if (eventCount >= LOG_EVENTS_MAX) {
        LOG_FLUSH();
    }
    const uint32 eventIndex = eventFirst + eventCount;

    uint32 writeIndex;
    if (eventCount == 0) {
        writeIndex = 0;
    }
    else {
        uint32 prevEventIndex;
        if (eventIndex == 0) {
            prevEventIndex = LOG_EVENTS_MAX - 1;
        }
        else {
            prevEventIndex = eventIndex - 1;
        }

        const LogEvent& prevEvent = logEvents[prevEventIndex];
        writeIndex = prevEvent.logStart + prevEvent.logSize;
        if (writeIndex >= LOG_BUFFER_SIZE) {
            writeIndex -= LOG_BUFFER_SIZE;
        }
    }

    uint32 freeSpace1, freeSpace2;
    freeSpace1 = LOG_BUFFER_SIZE - writeIndex;
    freeSpace2 = writeIndex;

    DEBUG_ASSERT(freeSpace1 != 0 || freeSpace2 != 0);

    va_list args;
    va_start(args, format);
    int logSize = stbsp_vsnprintf(buffer.data + writeIndex, freeSpace1, format, args);
    if (logSize < 0 || logSize >= (int)freeSpace1) {
        logSize = stbsp_vsnprintf(buffer.data, freeSpace2, format, args);
        if (logSize < 0 || logSize >= (int)freeSpace2) {
            // Not necessarily too big to write, freeSpace1 + freeSpace2 might be big enough
            // But this is easier and probably fine
            LOG_FLUSH(); // TODO we're dropping this one log :(
            return;
        }
        MemCopy(buffer.data + writeIndex, buffer.data, freeSpace1);
        MemMove(buffer.data, buffer.data + freeSpace1, freeSpace2);
    }
    va_end(args);

    eventCount += 1;
    LogEvent& event = logEvents[eventIndex];
    event.category = category;

    string fileString = ToNonConstString(ToString(file));
    fileString.size = MinUInt32(fileString.size, PATH_MAX_LENGTH);
    event.file.FromArray(fileString);
    event.line = line;

    string functionString = ToNonConstString(ToString(function));
    functionString.size = MinUInt32(functionString.size, LogEvent::FUNCTION_NAME_MAX);
    event.function.FromArray(functionString);

    event.logStart = writeIndex;
    event.logSize = logSize;
}
