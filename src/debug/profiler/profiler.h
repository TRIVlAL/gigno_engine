/*
    PROFILER
Allows you to monitor the performance of parts of your app.

Use static function Profiler::Begin to add a new profile scope, that will show in the debug window.abort
These scopes must exist inside a loop (usually, the core engine loop) and one profiler scope tree exists per thread.

Use static function Profiler::End to finish a scope. To every Begin call, there MUST be an End call.

When the first scope is done, the Profiling Server considers the Iteration of the loop done. That means that you should never 
Begin() and End() two scopes sceparatly inside the loop : The scopes should be enclosed by the outer scope that spans from
the beginning of the loop to the end.

Requirements : in features_usage.h, macro USE_PROFILER  = 1
*/

#ifndef PROFILER_H
#define PROFILER_H

#include "../../features_usage.h"

#include <vector>
#include <chrono>
#include <mutex>

namespace gigno {

    const size_t PROFILER_RESOLUTION = 500;
    const float PROFILER_FROM_NANOSECOND_CONVERTION = 1e-3;

    class Profiler {
    class ProfileThread;
    friend class ProfileThread;
    public:
        static void Begin(const char *name);
        static void End();

        static void DrawProfilerTab();

    #if USE_PROFILER

    private:
        static thread_local ProfileThread s_Thread;

        static std::mutex s_BindThreadMutex;
        static std::vector<ProfileThread *> s_BoundThreads;

        class ProfileThread {
        public:
            ProfileThread();
            ~ProfileThread();

            void Begin(const char *name);
            void End();

            void DrawUI();

        private:
            struct ProfileScope_t;

            void EndFrame();

            ProfileScope_t *m_RootScope{};

            bool m_StartedRootScope = false;

            int m_ThreadHash{};
            int m_ThreadID{};
        };

#endif
    };

    struct Profiler::ProfileThread::ProfileScope_t
    {
        ProfileScope_t(const char *name) : Name{name} {};
        ~ProfileScope_t();

        struct ProfileData_t
        {
            size_t CurrentIndex{};
            // Are in milliseconds.
            float Durations[PROFILER_RESOLUTION];

            // Current ceilling of the plot.
            size_t CurrentCeilling = 10.0f;

            // Used for updating the ceilling height dynamically.
            int ValuesOverCeillingCount = 0;
            float AverageValueOverCeilling = 0.0f;

            // The sum of all the durations in the m_Durations array.
            float RecentTotal = 0.0f;

            int FramesWithoutValuesOverCeilling = 0;

            // How many time did it begin since the last BeginFrame() call.
            int CallCountThisFrame = 0;
            float MaxTotalDuration = 0.0f;

            void UpdateCeilling(float mili_duration);
        };

        const char *Name{};

        // -1 means no active child.
        int ActiveChildIndex = -1;
        // Use pointer because ProfileScope_t is still considered an incomplete type... C++ amma right ?
        std::vector<ProfileScope_t> Children{};

        // Wether this scope has called begin this frame...
        bool HasRun = false;

        ProfileData_t Data{};

        ProfileData_t DataCopy{};

        void Start();
        void Stop();
        void EndFrame();

        void DrawUI(int depth, int thread_id, int thread_hash);

    private:
        std::chrono::high_resolution_clock::time_point m_StartTime{};
        float m_TotalDurationThisFrame = 0.0f;
        int m_CallCountThisFrame = 0;
    };
}


#endif