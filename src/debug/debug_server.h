#ifndef DEBUG_SERVER_H
#define DEBUG_SERVER_H

#include "../features_usage.h"

#include "profiling/profiling_server.h"
#include "console/console.h"

namespace gigno {


    const int MAX_MESSAGE_LENGTH = 255;

    class DebugServer {
    public:
        ProfilingServer *Profiler() { return &m_Profiler; }
        Console *GetConsole() { return &m_Console; }

        void OpenWindow() {
        #if USE_IMGUI && USE_DEBUG_SERVER
            m_ShowDebugWindow = true;
        #endif
        }
        void CloseWindow() {
        #if USE_IMGUI && USE_DEBUG_SERVER
            m_ShowDebugWindow = false;
        #endif
        }

        void Update();
    private:
        ProfilingServer m_Profiler{};
        Console m_Console{};
    #if USE_DEBUG_SERVER

        bool m_ShowDebugWindow = false;

        bool m_ShowConsoleWindow = false;
        bool m_ShowProfilerWindow = false;
        bool m_ShowEntityInspector = false;
    #endif
    };


}

#endif