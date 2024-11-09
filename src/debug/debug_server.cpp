
#include "debug_server.h"
#include "../application.h"
#include "imgui.h"


namespace gigno {

    void DebugServer::Update() {
    #if USE_DEBUG_SERVER && USE_IMGUI
        m_Profiler.EndFrame();

        if(m_ShowDebugWindow) {
            ImGui::SetNextWindowSizeConstraints(ImVec2{650.0f, 500.0f}, ImVec2{FLT_MAX, FLT_MAX});
            if(!ImGui::Begin("Debug Window", &m_ShowDebugWindow)) {
                ImGui::End();
            } else {
                #if !USE_CONSOLE
                ImGui::Text("Note : Console disabled in this build.");
                #endif
                #if !USE_PROFILER
                ImGui::Text("Note : Profiler disabled in this build.");
                #endif
                if(ImGui::BeginTabBar("##debug_tabs")) {
                    #if USE_CONSOLE
                    if(ImGui::BeginTabItem("Console")) {
                        if(Console *cons = Console::Singleton()) {
                            cons->DrawConsoleTab();
                        }
                        ImGui::EndTabItem();
                    }
                    #endif
                    #if USE_PROFILER
                    if(ImGui::BeginTabItem("Profiler")) {
                        m_Profiler.DrawProfilerTab();
                        ImGui::EndTabItem();
                    }
                    #endif
                    if(ImGui::BeginTabItem("Inspector")) {
                        if(Application* app = Application::Singleton()) {
                            app->GetEntityServer()->DrawEntityInspectorTab();
                        }
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }
                ImGui::End();
            }
        }

        m_Profiler.StartFrame();
    #endif
    }
}