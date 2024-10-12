#include "profiling_server.h"
#include <iostream>
#include "../../error_macros.h"
#include <algorithm>
#include "imgui.h"
#include "../../application.h"

namespace gigno {

    ProfilingServer::ProfilingServer() {
    }

    ProfilingServer::~ProfilingServer() {

    }

    void ProfilingServer::Begin(const std::string &name) {
        #if USE_IMGUI
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
        #endif
    }

    void ProfilingServer::End() {
        #if USE_IMGUI
        ASSERT(m_pActiveScope);

        m_pActiveScope->Stop();
        m_pActiveScope = m_pActiveScope->End();
        #endif
    }

    void ProfilingServer::EndFrame() {
#if USE_IMGUI
        for(ProfileScope &scope : m_RootScopes) {
            scope.EndFrame();
        }


        if(m_ShowProfilerWindow) {
            if(!ImGui::Begin("Profiler", &m_ShowProfilerWindow, ImGuiWindowFlags_::ImGuiWindowFlags_NoResize)) {
                ImGui::End();
                return;
            }
            ImGui::SetWindowSize(ImVec2{650.0f, 500.0f});

            ImGui::Text("The profiler allows you to monitor the timing and performance of\n"
                        "core parts of your code.");


            for(ProfileScope scope : m_RootScopes) {
                scope.DrawUI();
            }

            ImGui::End();
        }

        for(ProfileScope& scope : m_RootScopes) {
            scope.StartFrame();
        }
#endif
    }

}