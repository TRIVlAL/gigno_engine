#include "profiling_server.h"
#include <iostream>
#include "../error_macros.h"
#include <algorithm>
#include "imgui.h"

namespace gigno {

    ProfilingServer::ProfilingServer() {
    }

    ProfilingServer::~ProfilingServer() {

    }

#if USE_IMGUI
    void ProfilingServer::Begin(const std::string &name) {
        if (m_pActiveScope) {
            m_pActiveScope = m_pActiveScope->BeginChild(name);
        } else {
            for(int i = 0; i < m_RootScopes.size(); i++) {
                if(m_RootScopes[i].GetName() == name) {
                    m_pActiveScope = &m_RootScopes[i];
                    m_pActiveScope->Start();
                    return;
                }
            }

            // No scope with the same name exist.
            m_pActiveScope = &m_RootScopes.emplace_back(name);
            m_pActiveScope->Start();
        }
    }

    void ProfilingServer::End() {
        ASSERT(m_pActiveScope);

        m_pActiveScope->Stop();
        m_pActiveScope = m_pActiveScope->End();
    }

    void ProfilingServer::EndFrame() {
        for(ProfileScope &scope : m_RootScopes) {
            scope.EndFrame();
        }

        ImGui::ShowDemoWindow();

        if(!m_ShowProfilerWindow) {
            return;
        }
        if(!ImGui::Begin("Profiler", &m_ShowProfilerWindow, ImGuiWindowFlags_::ImGuiWindowFlags_NoResize)) {
            ImGui::End();
            return;
        }
        ImGui::SetWindowSize(ImVec2{650.0f, 500.0f});

        for(ProfileScope scope : m_RootScopes) {
            scope.DrawUI();
        }

        ImGui::End();
    }
#endif

}