#include "error_macros.h"
#include <numeric>
#include <chrono>
#include "entities/dome_camera.h"
#include "entities/spinner.h"
#include "entities/lights/directional_light.h"
#include "entities/lights/point_light.h"
#include "entities/lights/environment_light.h"
#include "features_usage.h"
#include "stringify.h"
#include "debug/console/convar.h"
#include "debug/profiler/profiler.h"
#include "physics/rigid_body.h"
#include "test_entity.h"
#include "rendering/gui.h"
#include "application.h"
#include "algorithm/arena.h"

namespace gigno {

	CONVAR(const char *, start_map, "assets/maps/demo_05.map", "the first loaded map when the app stats.")

	Application::Application() :
		m_CurrentMapFilepath{convar_start_map} {

			ASSERT_MSG(!s_Instance, "Multiple applications created !");
			s_Instance = this;

			m_RenderingServer.Init(1000, 1000, "Gigno Engine Demo");
			m_AudioServer.Init();
			m_PhysicServer.Init();

			m_NextMapFilepath = m_CurrentMapFilepath;
		}

	Application::~Application() {
		if(s_Instance == this) {
			s_Instance = nullptr;
		}
	}

	Application *Application::Singleton() {
		return s_Instance;
	}

	AppExitCode_t Application::run() {
		Console::StartFileLogging();

		ASSERT_MSG_V(glfwInit(), EXIT_FAILED_GLFW, "GLFW Failed to init");

		auto last_update_time = std::chrono::steady_clock::now();

		auto start_time = std::chrono::steady_clock::now();
		
		while (m_CurrentStatus == CONTINUE) {
			Profiler::Begin("Main Loop");

			if(m_ShouldLoadMap) {
				Console::LogInfo("Loading map file '%s'", m_NextMapFilepath.c_str());
				std::ifstream map_stream{m_NextMapFilepath};
				if(!map_stream) {
					Console::LogWarning("Failed to open map file !");
				} else {
					if (!m_EntityServer.LoadFromFile(map_stream)) {
						Console::LogWarning("Error when parsing map file !");
						//Fallback to the last map.
						std::ifstream old_filestream{m_CurrentMapFilepath};
						m_EntityServer.LoadFromFile(old_filestream);
					}
				}
				m_ShouldLoadMap = false;
				m_CurrentMapFilepath = m_NextMapFilepath;
			}
			

			m_RenderingServer.PollEvents();
			m_InputServer.UpdateInput();

			if(m_InputServer.GetKeyDown(KEY_M)) {
				m_ShowMainUIWindow = true;
			}
			DrawMainUIWindow();

			auto current_time = std::chrono::steady_clock::now();
			std::chrono::duration<float> delta_time = current_time - last_update_time;
			delta_time = std::chrono::duration<float>{glm::min(delta_time.count(), MAX_FRAME_TIME)};
			delta_time = std::chrono::duration_cast<std::chrono::microseconds>(delta_time);

			last_update_time = current_time;

			m_EntityServer.Tick(delta_time.count() * 10e-1f); // For some reason, it seems that to get second we need
															  //  to multiply by 10e-1f and not the expected 10e-6f !
															  //  Related to issue #2

			Profiler::Begin("Render Frame");
			m_RenderingServer.Render();

			Profiler::End();

			if(m_RenderingServer.WindowShouldClose()) {
				SetExit(EXIT_SIMPLE);
			}

			Profiler::End(); //Main Loop

			Debug()->Update();
		}

		m_EntityServer.UnloadMap();

		return m_CurrentStatus;
	}

	void Application::DrawMainUIWindow() {
		if(!m_ShowMainUIWindow) {
			return;
		}
	#if USE_IMGUI
			ImGui::Begin("Gigno Engine", &m_ShowMainUIWindow);
			if(m_InputServer.GetKeyDown(KEY_M)) {
				if(ImGui::IsWindowCollapsed()) {
					ImGui::SetWindowCollapsed(false);
				}
			}

			ImGui::Text("Welcome to the Gigno Engine !");

		#if USE_DEBUG_SERVER
			ImGui::SeparatorText("Debuging");
			if(ImGui::Button("Debug Window")) {
				m_DebugServer.OpenWindow();
			}
		#endif


		ImGui::End();
	#endif
	}
}

void gigno::Application::SetExit(AppExitCode_t exit) {
	// Error codes take priority over SUCCESS.
	if(exit != CONTINUE && m_CurrentStatus == CONTINUE || m_CurrentStatus == SUCCESS) {
		m_CurrentStatus = exit;
	}
}

void gigno::Application::LoadMap(const char *filepath)
{
    m_ShouldLoadMap = true;
	m_NextMapFilepath = filepath;
}
