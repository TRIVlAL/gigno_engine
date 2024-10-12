#ifndef DEBUG_SERVER_H
#define DEBUG_SERVER_H

#include "profiling/profiling_server.h"
#include "console.h"

#include <cstdio>
#include <stdarg.h>

namespace gigno {

    const int MAX_MESSAGE_LENGTH = 255;

    class DebugServer {
    public:
        ProfilingServer *Profiler() { return &m_Profiler; }

        void LogInfo(const char *fmt, ...) {
            va_list args;
            va_start(args, fmt);
            int result = vsnprintf(m_PrintBuffer, MAX_MESSAGE_LENGTH, fmt, args);
            m_Console.Log(m_PrintBuffer, CONSOLE_MESSAGE_INFO);
        }
        void LogWarning(const char *fmt, ...) {
            va_list args;
            va_start(args, fmt);
            int result = vsnprintf(m_PrintBuffer, MAX_MESSAGE_LENGTH, fmt, args);
            m_Console.Log(m_PrintBuffer, CONSOLE_MESSAGE_WARN);
        }
        void LogError(const char *fmt, ...) {
            va_list args;
            va_start(args, fmt);
            int result = vsnprintf(m_PrintBuffer, MAX_MESSAGE_LENGTH, fmt, args);
            m_Console.Log(m_PrintBuffer, CONSOLE_MESSAGE_ERR);
        }

    private:
        ProfilingServer m_Profiler{};
        Console m_Console{};

        char m_PrintBuffer[MAX_MESSAGE_LENGTH];
    };

}

#endif