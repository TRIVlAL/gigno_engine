#include "error_macros.h"
#include <numeric>
#include <chrono>
#include "entities/dome_camera.h"
#include "entities/spinner.h"
#include "entities/lights/directional_light.h"
#include "entities/lights/point_light.h"
#include "entities/lights/environment_light.h"
#include "core_macros.h"
#include <thread>

namespace gigno {

	Application::Application(int winw, int winh, const char *title, const std::string &vertShaderPath, const std::string &fragShaderPath) :
		m_DebugServer{},
		m_InputServer{},
		m_RenderingServer{ winw, winh, title, &m_InputServer, vertShaderPath, fragShaderPath },
		m_EntityServer{} {

		}

	Application::~Application() {}

	EntityServer *Application::GetEntityServer() {
		return &m_EntityServer;
	}

	Application *Application::MakeApp() {
		Application *app = new Application(1000, 1000, "Gigno Engine Demo", "shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv");
		s_Instance = app;
		return app;
	}

	Application *Application::Singleton() {
		return s_Instance;
	}

	int Application::run() {

		ASSERT_MSG_V(glfwInit(), 1, "GLFW Failed to init");

		RenderedEntity first{ModelData_t::FromObjFile("models/smooth_vase.obj")};
		first.Transform.translation = glm::vec3{ 0.0f, 0.0f, 0.0f };
		first.Transform.scale = glm::vec3{ 3.0f, -1.5f, 3.0f };
		first.Transform.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
		float rot = 0;
		float y = -5.0f;

		RenderedEntity second{ModelData_t::FromObjFile("models/flat_vase.obj")};
		second.Transform.translation = glm::vec3{ 1.0f, 0.5f, 1.0f };
		second.Transform.scale = glm::vec3{3.0f, 3.0f, 3.0f};
		second.Transform.rotation = glm::vec3(glm::radians(-90.0f), glm::radians(180.0f), glm::radians(-50.0f));

		Spinner third{ModelData_t::FromObjFile("models/colored_cube.obj")};
		third.Transform.translation = glm::vec3{-0.5f, 0.75f, -0.5f};
		third.Transform.scale = glm::vec3{.2f, .2f, .2f};
		third.Transform.rotation = glm::vec3(0.0f, glm::radians(55.0f), 0.0f);
		third.Speed = glm::two_pi<float>();

		Spinner fourth{ModelData_t::FromObjFile("models/smooth_vase.obj")};
		fourth.Transform.translation = glm::vec3{0.5f, 0.0f, 0.5f};
		fourth.Transform.scale = glm::vec3{3.0f, 2.0f, 3.0f};
		fourth.Transform.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
		second.Name = "Upside-down";
		fourth.Speed = 0.5f * glm::two_pi<float>();

		RenderedEntity fifth{ModelData_t::FromObjFile("models/smooth_vase.obj")};
		fifth.Transform.translation = glm::vec3{0.5f, 2.0f, 0.5f};
		fifth.Transform.scale = glm::vec3{3.0f, 2.0f, 3.0f};
		fifth.Transform.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
		second.Name = "Upside-down above";

		DomeCamera camera(10.0f);
		camera.SetPerspectiveProjection(glm::radians(50.0f), m_RenderingServer.GetAspectRatio(), -0.05f, 1.0f);
		camera.Transform.translation = { 0.0f, 0.0f, -3.0f };
		camera.Transform.rotation.y = 0;
		camera.SetTarget( (first.Transform.translation + second.Transform.translation + third.Transform.translation + fourth.Transform.translation) * 0.25f );
		camera.Name = "My Camera";


		DirectionalLight sun2;
		sun2.Intensity = 0.5f;
		sun2.Direction = glm::normalize(glm::vec3{-1.0f, 0.0f, 2.0f});

		PointLight bulb;
		bulb.Transform.translation = glm::vec3{0.5f,0.5f, 0.5f};
		bulb.Intensity = .5f;

		EnvironmentLight env;
		env.intensity = 0.02f;

		auto lastUpdateTime = std::chrono::steady_clock::now();

		Debug()->GetConsole()->StartFileLogging();
		m_EntityServer.Start();

			Debug()->GetConsole()->LogInfo("Hello %s", "mariooooo");
			Debug()->GetConsole()->LogWarning("This format should work : %s", "HELLO HELLO");
			Debug()->GetConsole()->LogError("ERR ERR ERR %f, 2%%", 52.3f);

			Debug()->GetConsole()->LogError("error message unformated !!!");

		while (!m_RenderingServer.WindowShouldClose()) {
			Debug()->Profiler()->Begin("Main Loop");

			static int i = 0;
			i++;
			Debug()->GetConsole()->LogInfo("Hello %d", i);


			m_RenderingServer.PollEvents();
			m_InputServer.UpdateInput();

			if(m_InputServer.GetKeyDown(KEY_M)) {
				m_ShowMainUIWindow = true;
			}
			DrawMainUIWindow();

			auto currentTime = std::chrono::steady_clock::now();
			
			std::chrono::duration<float> deltaTime = currentTime - lastUpdateTime;
			deltaTime = std::chrono::duration<float>{ glm::min(deltaTime.count(), MAX_FRAME_TIME) };
			deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(deltaTime);

			lastUpdateTime = currentTime;

			m_RenderingServer.DrawLineGradient(glm::vec3{0.0f, 1.0f, 0.0f}, glm::vec3{0.0f, 1.0f, 1.0f}, glm::vec3{1.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 0.0f, 1.0f}, UNIQUE_NAME);
			m_RenderingServer.DrawLineGradient(glm::vec3{0.0f, 1.0f, 1.0f}, glm::vec3{0.5f, 1.0f, 0.5f}, glm::vec3{0.0f, 0.0f, 1.0f}, glm::vec3{0.0f, 1.0f, 0.0f}, UNIQUE_NAME);
			m_RenderingServer.DrawLineGradient(glm::vec3{0.5f, 1.0f, 0.5f}, glm::vec3{0.0f, 1.0f, 0.0f}, glm::vec3{0.0f, 1.0f, 0.0f}, glm::vec3{1.0f, 0.0f, 0.0f}, UNIQUE_NAME);
			m_RenderingServer.DrawPoint(bulb.Transform.translation, glm::vec3{1.0f, 1.0f, 1.0f}, UNIQUE_NAME);
			
			Debug()->Profiler()->Begin("Update Entities");
			m_EntityServer.Tick(deltaTime.count() * 10e-1f); //For some reason, it seems that to get second we need
															 // to multiply by 10e-1f and not the expected 10e-6f !
															 // Related to issue #2
			Debug()->Profiler()->End();
			
			Debug()->Profiler()->Begin("Render Frame");
			m_RenderingServer.Render();
			Debug()->Profiler()->End();

			Debug()->Profiler()->End(); //Main Loop

			Debug()->Update();
		}

		m_RenderingServer.Finalize();

		return 0;
	}

	void Application::ShutdownApp() {
		Application *app = s_Instance;
		s_Instance = nullptr;
		delete(app);
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