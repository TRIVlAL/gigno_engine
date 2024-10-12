#include "profile_scope.h"

#if USE_IMGUI

#include <algorithm>
#include <iostream>
#include "../../vendor/imgui/imgui.h"
#include <math.h>

namespace gigno {

    ProfileScope::ProfileScope(const std::string &name) : m_Name{name} {
        for(int i = 0; i < PROFILE_SCOPE_RESOLUTION; i++) {
            m_Durations[i] = 0.0f;
        }
    }

    ProfileScope::~ProfileScope(){
    }

    void ProfileScope::Start() {
        m_StartTime = std::chrono::high_resolution_clock::now();
    }

    void ProfileScope::Stop() {
        auto endTime = std::chrono::high_resolution_clock::now();

        auto actualStart = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTime).time_since_epoch();
        auto actualEnd = std::chrono::time_point_cast<std::chrono::microseconds>(endTime).time_since_epoch();

        auto duration = actualEnd - actualStart;

        m_CurrentFrameDuration += duration;
        m_CallCountThisFrame++;
        if(duration.count() > m_MaxPerCallDuration){
            m_MaxPerCallDuration = duration.count();
        }
    }

    void ProfileScope::EndFrame() {
        for(auto &child : m_Children) {
            child.EndFrame();
        }
        
        if(m_CurrentDurationIndex == PROFILE_SCOPE_RESOLUTION - 1) {
            m_CurrentDurationIndex = 0;
        } else {
            m_CurrentDurationIndex++;
        }

        float dur = m_CurrentFrameDuration.count();
        m_RecentTotal -= m_Durations[m_CurrentDurationIndex];
        m_RecentTotal += dur;
        m_Durations[m_CurrentDurationIndex] = dur;

        if(dur > m_MaxValue) {
            m_MaxValue = dur;
        }

        UpdateCeilling(dur);

        m_CurrentFrameDuration = std::chrono::microseconds(0);
    }

    void ProfileScope::UpdateCeilling(float duration) {
        if(duration > m_CurrentCeilling) {
            float total = m_AverageValueOverCeilling * m_ValuesOverCeillingCount;
            total += duration;
            m_ValuesOverCeillingCount++;
            m_FramesWithoutValuesOverCeilling = 0;
            m_AverageValueOverCeilling = total / m_ValuesOverCeillingCount;
        } else {
            m_FramesWithoutValuesOverCeilling++;
        }
        if(m_ValuesOverCeillingCount > 5) {
            m_CurrentCeilling = m_AverageValueOverCeilling;
            m_ValuesOverCeillingCount = 0;
            m_AverageValueOverCeilling = 0.0f;
        }
        if(m_RecentTotal / PROFILE_SCOPE_RESOLUTION < m_CurrentCeilling * 0.35f) {
            m_CurrentCeilling -= (m_ValuesOverCeillingCount > 0? .1f : m_CurrentCeilling * 0.001f + 3.0f);
        }
        if(m_FramesWithoutValuesOverCeilling > 4000) {
            m_ValuesOverCeillingCount = 0;
        }
    }

    void ProfileScope::StartFrame() {
        for(auto& child : m_Children) {
            child.StartFrame();
        }
        m_MaxPerCallDuration = 0.0f;
        m_CallCountThisFrame = 0;

    }

    ProfileScope *ProfileScope::BeginChild(const std::string &name) {
        for(int i = 0; i < m_Children.size(); i++) {
            if(m_Children[i].GetName() == name) {
                m_Children[i].Start();
                m_Children[i].SetEscape(this);
                return &m_Children[i];
            }
        }

        // No child with the same name exist.
        m_Children.emplace_back(name).SetEscape(this);
        m_Children.front().Start();
        return &m_Children.front();
    }

    ProfileScope *ProfileScope::End() {
        return m_pEscape;
    }

    void ProfileScope::DrawUI(int childDepth) {
        if(ImGui::TreeNode(m_Name.data())) {
            float width = 500 - (float)childDepth * ImGui::GetTreeNodeToLabelSpacing();
            float height = std::max(100.0f - (float)childDepth * 20.0f, 40.0f);
            ImGui::PlotLines("##Durations", m_Durations, PROFILE_SCOPE_RESOLUTION, m_CurrentDurationIndex, nullptr, 0.0f, 
                            m_CurrentCeilling + 1.0f, ImVec2{width, height});
            ImGui::SameLine();
            float posX = ImGui::GetCursorPosX();
            ImGui::Text("%d us", (int)m_CurrentCeilling);

            ImGui::SameLine();
            ImGui::SetCursorPosX(posX);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + height/2 -5.0f);
            ImGui::Text("%d us", (int)m_Durations[m_CurrentDurationIndex]);

            ImGui::SameLine();
            ImGui::SetCursorPosX( posX);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() +  height - 10.0f);
            ImGui::Text("%d us", (int)0.0f);

            ImGui::Text("Recent Average :");
            ImGui::SameLine(); 
            ImGui::SetCursorPosX(width/2);
            int avrg = (int)(m_RecentTotal / PROFILE_SCOPE_RESOLUTION);
            ImGui::Text("%d us", avrg);

            ImGui::Text("Max Value :");
            ImGui::SameLine();
            ImGui::SetCursorPosX(width/2);
            ImGui::Text("%d us", (int)m_MaxValue);

            if(m_CallCountThisFrame == 1) {
                ImGui::Text("Called once.");
            } else {
                ImGui::Text("Called %d times this frame", m_CallCountThisFrame);
                ImGui::Text("Average per call :");
                ImGui::SameLine(); 
                ImGui::SetCursorPosX(width/2);
                int avrg = (int)(m_Durations[m_CurrentDurationIndex] / m_CallCountThisFrame);
                ImGui::Text("%d us", avrg);

                ImGui::Text("Max Duration :");
                ImGui::SameLine(); 
                ImGui::SetCursorPosX(width/2);
                ImGui::Text("%d us", (int)m_MaxPerCallDuration);
            }

            if(m_Children.size() > 0)
            {
                ImGui::SeparatorText("Contains :");
            }
            for(ProfileScope& child : m_Children) {
                child.DrawUI(childDepth + 1);
            }
            ImGui::TreePop();
        }
    }

}

#endif //USE_IMGUI