#ifndef PROFILE_SCOPE_H
#define PROFILE_SCOPE_H

#include "../core_macros.h"
#if USE_IMGUI

#include <string>
#include <vector>
#include <chrono>
#include <limits>

namespace gigno
{
    const int PROFILE_SCOPE_RESOLUTION = 500;

    class ProfileScope
    {
    public:

        ProfileScope(const std::string &name);
        ~ProfileScope();

        const std::string &GetName() const { return m_Name; }

        void Start();
        void Stop();

        void EndFrame();

        ProfileScope *BeginChild(const std::string &name);

        ProfileScope *End();

        void DrawUI(int childDepth = 0);

    private:
        void SetEscape(ProfileScope *escape) { m_pEscape = escape; }

        std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTime;

        std::chrono::microseconds m_CurrentFrameDuration{0};

        float m_Durations[PROFILE_SCOPE_RESOLUTION];
        int m_CurrentDurationIndex = 0;

        float m_CurrentCeilling = 10.0f;
        int m_ValuesOverCeillingCount = 0;
        float m_AverageValueOverCeilling = 0.0f;

        float m_CurrentTotal = 0.0f;

        float m_MaxValue = 0.0f;

        int m_FramesWithoutValuesOverCeilling = 0;

        std::string m_Name;

        std::vector<ProfileScope> m_Children;

        ProfileScope *m_pEscape{};
    };
}

#endif //USE_IMGUI

#endif