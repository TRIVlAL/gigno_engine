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
    // Number of frame for which we keep the duration data.
    const int PROFILE_SCOPE_RESOLUTION = 500;

    class ProfileScope
    {
    public:

        ProfileScope(const std::string &name);
        ~ProfileScope();

        const std::string &GetName() const { return m_Name; }

        /*
        @brief Begin to monitor a new call.
        */
        void Start();
        void Stop();

        /*
        @brief Called once per frame before DrawUI.
        */
        void EndFrame();
        /*
        @brief Called once per frame after DrawUI.
        */
        void StartFrame();

        ProfileScope *BeginChild(const std::string &name);

        ProfileScope *End();

        /*
        Must already be in an ImGui scope.
        */
        void DrawUI(int childDepth = 0);

    private:
        void SetEscape(ProfileScope *escape) { m_pEscape = escape; }
        
        // Update plot ceilling height (if its too low/high)
        void UpdateCeilling(float duration);

        //Start time of the current call.
        std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTime;

        std::chrono::microseconds m_CurrentFrameDuration{0};

        float m_Durations[PROFILE_SCOPE_RESOLUTION];
        int m_CurrentDurationIndex = 0;

        // CUrrent ceilling of the plot.
        float m_CurrentCeilling = 10.0f;

        // Used for updating the ceilling height dynamically.
        int m_ValuesOverCeillingCount = 0;
        float m_AverageValueOverCeilling = 0.0f;

        // The sum of all the durations in the m_Durations array.
        float m_RecentTotal = 0.0f;

        float m_MaxValue = 0.0f;

        int m_FramesWithoutValuesOverCeilling = 0;

        // How many time did it begin since the last BeginFrame() call.
        int m_CallCountThisFrame = 0;
        float m_MaxPerCallDuration = 0.0f;

        std::string m_Name;

        std::vector<ProfileScope> m_Children;

        ProfileScope *m_pEscape{}; //The Scope that should take control
                                    // when end (the parent).
    };
}

#endif //USE_IMGUI

#endif