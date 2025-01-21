#include "../features_usage.h"

#if USE_CONSOLE && USE_IMGUI
#include "../debug/console/command.h"
#include "../application.h"
#include "../rendering/gui.h"

namespace gigno {
    namespace {
        bool s_PhysRemoteToggle = false;
        int s_PhysRemoteCurrSpeed = 0;
    };

    CONSOLE_COMMAND_HELP_UPDATE(phys_remote, "toggles the physics remote window, which allows to pause/play/slowdown the physics simulation.") {
        s_PhysRemoteToggle = !s_PhysRemoteToggle;
    }

    void phys_remote_update(float dt) {
        PhysicServer *phys = Application::Singleton()->GetPhysicServer();

        if(!s_PhysRemoteToggle) {
            return;
        }

        if(!ImGui::Begin("Physics Remote", &s_PhysRemoteToggle, ImGuiWindowFlags_NoResize)) {
            ImGui::End();
            return;
        }
        ImGui::SetWindowSize(ImVec2{260.0f, 100.0f});
        ImGui::Separator();
        ImGui::NewLine();

        if (ImGui::Button(s_PhysRemoteCurrSpeed == 0   ? "1.0x"
                          : s_PhysRemoteCurrSpeed == 1 ? "0.5x"
                          : s_PhysRemoteCurrSpeed == 2 ? "0.2x"
                                                       : "--x",
                            ImVec2{60.0f, 0.0f}))
        {
            const char *call = (s_PhysRemoteCurrSpeed == 0 ? "phys_timescale 0.5"
                                                    : s_PhysRemoteCurrSpeed == 1 ? "phys_timescale 0.2"
                                                    : s_PhysRemoteCurrSpeed == 2 ? "phys_timescale 1.0" : "na");
            Console::CallCommand(call);
            s_PhysRemoteCurrSpeed = s_PhysRemoteCurrSpeed == 2 ? 0 : s_PhysRemoteCurrSpeed + 1;
        }

        ImGui::SameLine(0.0f, 10.0f);

        if(ImGui::Button(phys->m_Pause?"Play":"Pause", ImVec2{100.0f, 0.0f})) {
            phys->m_Pause = !phys->m_Pause;
        }

        ImGui::SameLine(0.0f, 10.0f);
        ImGui::BeginDisabled(!phys->m_Pause);
        if (ImGui::Button("Step", ImVec2{60.0f, 0.0f})) {
            phys->m_Step = true;
        }
        ImGui::EndDisabled();

        ImGui::NewLine();
        ImGui::Separator();

        ImGui::End();
    }
}
#endif