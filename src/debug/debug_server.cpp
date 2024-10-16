
#include "debug_server.h"
#include "../application.h"
#include "imgui.h"


namespace gigno {

    void DebugServer::Update() {
    #if USE_DEBUG_SERVER && USE_IMGUI
        if(m_ShowDebugWindow) {
        if(!ImGui::Begin("Debug Window", &m_ShowDebugWindow)) {
            ImGui::End();
        } else {
            ImGui::TextWrapped("Everything needed for debugging your app/the engine is found here. "
            "You can disable all the debug features by turning off USE_DEBUG_SERVER in 'core_macros.h'.");
            ImGui::Separator();
            #if !USE_PROFILER
            ImGui::BeginDisabled(true);
            #endif
            if(ImGui::Button("Profiler")) {
                m_ShowProfilerWindow = true;
            }
            #if !USE_PROFILER
            ImGui::EndDisabled();
            ImGui::SameLine(); ImGui::TextColored(ImVec4{0.8, 0.0, 0.0, 1.0}, "Profiler disabled in this build.");
            #endif
            #if !USE_CONSOLE
            ImGui::BeginDisabled(true);
            #endif
            if(ImGui::Button("Console")) {
                m_ShowConsoleWindow = true;
            }
            #if !USE_CONSOLE
            ImGui::EndDisabled();
            ImGui::SameLine(); ImGui::TextColored(ImVec4{0.8, 0.0, 0.0, 1}, "Console disabled in this build.");
            #endif
            if(ImGui::Button("Entity Inspector")) {
                m_ShowEntityInspector = true;
            }
            if(Application *app = Application::Singleton()) {
                ImGui::SeparatorText("Rendering");

                if(ImGui::Checkbox("Fullbright", app->GetRenderer()->Fullbright()));

                if(ImGui::CollapsingHeader("Debug Drawings")) {
                #if !USE_DEBUG_DRAWING
                    ImGui::Text("Debug Drawing is disabled !");
                    ImGui::Text("Enable Debug Drawing in core_macros.h.");
                #else
                    ImGui::Checkbox("Show Debug Drawings", &app->GetRenderer()->ShowDD);
                    ImGui::BeginDisabled(!app->GetRenderer()->ShowDD);
                        ImGui::Checkbox("Show Points", &app->GetRenderer()->ShowDDPoints); ImGui::SameLine(); ImGui::Checkbox("Show Lines", &app->GetRenderer()->ShowDDLines);
                    ImGui::EndDisabled();
                #endif
                }
            }
            ImGui::End();
        }
        }

        m_Console.DrawConsoleWindow(&m_ShowConsoleWindow);

        if(Application* app = Application::Singleton()) {
            app->GetEntityServer()->DrawEntityInspector(&m_ShowEntityInspector);
        }

        m_Profiler.EndFrame();
        m_Profiler.DrawProfilerWindow(&m_ShowProfilerWindow);
        m_Profiler.StartFrame();
    #endif
    }
}