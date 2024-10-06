#ifndef PROFILING_SERVER_H
#define PROFILING_SERVER_H

#include "profile_scope.h"
#include <vector>
#include <unordered_map>
#include "../core_macros.h"

namespace gigno {

    class ProfilingServer {
    public:
        ProfilingServer();
        ~ProfilingServer();

        void Begin(const std::string &uniqueName);
        void End();
    #if USE_IMGUI

        void EndFrame();

        void OpenWindow() {m_ShowProfilerWindow = true;}
        void CloseWindow() {m_ShowProfilerWindow = false;}
        void ToggleWindow() {m_ShowProfilerWindow = !m_ShowProfilerWindow;}
        
    private:
        std::vector<ProfileScope> m_RootScopes;

        ProfileScope *m_pActiveScope = nullptr;

        bool m_ShowProfilerWindow = false;

    #endif //USE_IMGUI
    };

}

#endif
