#ifndef DEBUG_SERVER_H
#define DEBUG_SERVER_H

#include "profiling/profiling_server.h"
#include "console.h"

namespace gigno {

    const int MAX_MESSAGE_LENGTH = 255;

    class DebugServer {
    public:
        ProfilingServer *Profiler() { return &m_Profiler; }
        Console *GetConsole() { return &m_Console; }

    private:
        ProfilingServer m_Profiler{};
        Console m_Console{};

        char m_PrintBuffer[MAX_MESSAGE_LENGTH];
    };

}

#endif