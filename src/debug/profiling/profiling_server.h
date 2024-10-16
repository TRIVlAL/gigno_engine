#ifndef PROFILING_SERVER_H
#define PROFILING_SERVER_H

#include "profile_scope.h"
#include <vector>
#include <unordered_map>
#include "../../core_macros.h"
#include <string>

namespace gigno {

    /*
        System allowing you to profile/analyse the performance of part of your code.

        Usage:
            * Requirements :
              * !!! Requires ImGui to work !!! Set USE_IMGUI flag to 1 in core_macros.h !!!
            * Key Methods :
              * Begin(...) : Begins a Child Profiling Scope in the hierarchy Its data will be 
                            In the profiler window.
              * End() : Stops the current Profiling Scope (required !)
              * EndFrame() : Must be called at the end of every frame of the main loop.
    */
    class ProfilingServer {
    public:
        ProfilingServer();
        ~ProfilingServer();

        /*
        @brief Begins a Profile Scope. Its duration and various profiling info will be shown
             In the Profiler window.
             Should always have a matching End() call.
        @param uniqueName must be different to any other scope already opened as child of the
                          current scope.
        */
        void Begin(const std::string &uniqueName);
        void End();

        void EndFrame();
        void DrawProfilerWindow(bool *open);
        void StartFrame();

    #if  USE_IMGUI && USE_DEBUG_SERVER && USE_PROFILER
    private:
        // The scopes work as a hierarchy : Every scope have zero or more children.
        std::vector<ProfileScope> m_RootScopes;

        ProfileScope *m_pActiveScope = nullptr;

    #endif
    };

}

#endif
