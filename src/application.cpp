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
#include "physics/rigid_body.h"
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
		Debug()->GetConsole()->StartFileLogging();

		ASSERT_MSG_V(glfwInit(), 1, "GLFW Failed to init");

		/*
		RenderedEntity first{ModelData_t::FromObjFile("models/smooth_vase.obj")};
		first.Transform.Position = glm::vec3{ 0.0f, 0.0f, 0.0f };
		first.Transform.Scale = glm::vec3{ 3.0f, -1.5f, 3.0f };
		first.Transform.Rotation = glm::vec3(0.0f, 0.0f, 0.0f);
		float rot = 0;
		float y = -5.0f;

		RenderedEntity second{ModelData_t::FromObjFile("models/flat_vase.obj")};
		second.Transform.Position = glm::vec3{ 1.0f, 0.5f, 1.0f };
		second.Transform.Scale = glm::vec3{3.0f, 3.0f, 3.0f};
		second.Transform.Rotation = glm::vec3(glm::radians(-90.0f), glm::radians(180.0f), glm::radians(-50.0f));

		Spinner third{ModelData_t::FromObjFile("models/colored_cube.obj")};
		third.Transform.Position = glm::vec3{-0.5f, 0.75f, -0.5f};
		third.Transform.Scale = glm::vec3{.2f, .2f, .2f};
		third.Transform.Rotation = glm::vec3(0.0f, glm::radians(55.0f), 0.0f);
		third.Speed = glm::two_pi<float>();

		Spinner fourth{ModelData_t::FromObjFile("models/smooth_vase.obj")};
		fourth.Transform.Position = glm::vec3{0.5f, 0.0f, 0.5f};
		fourth.Transform.Scale = glm::vec3{3.0f, 2.0f, 3.0f};
		fourth.Transform.Rotation = glm::vec3(0.0f, 0.0f, 0.0f);
		second.Name = "Upside-down";
		fourth.Speed = 0.5f * glm::two_pi<float>();

		RenderedEntity fifth{ModelData_t::FromObjFile("models/smooth_vase.obj")};
		fifth.Transform.Position = glm::vec3{0.5f, 2.0f, 0.5f};
		fifth.Transform.Scale = glm::vec3{3.0f, 2.0f, 3.0f};
		fifth.Transform.Rotation = glm::vec3(0.0f, 0.0f, 0.0f);
		second.Name = "Upside-down above";
		*/

		DomeCamera camera(10.0f);
		camera.SetPerspectiveProjection(glm::radians(50.0f), m_RenderingServer.GetAspectRatio(), -0.05f, 1.0f);
		camera.Transform.Position = { 0.0f, 0.0f, -10.0f };
		camera.Transform.Rotation.y = 0;
		camera.SetTarget( glm::vec3{0.0f} );
		camera.Name = "My Camera";


		DirectionalLight sun2;
		sun2.Intensity = 0.5f;
		sun2.Direction = glm::normalize(glm::vec3{-1.0f, 0.0f, 2.0f});

		PointLight bulb;
		bulb.Transform.Position = glm::vec3{0.5f,0.5f, 0.5f};
		bulb.Intensity = .5f;

		EnvironmentLight env;
		env.intensity = 0.02f;

		RigidBody phys_cube{ModelData_t::FromObjFile("models/colored_cube.obj")};
		phys_cube.Transform.Position = glm::vec3{-4.0f, 2.0f, 0.0f};
		phys_cube.Transform.Scale = glm::vec3{0.2, 0.2f, 0.2f};
		phys_cube.TestingInterpolateType = 0;

		RigidBody phys_cube2{ModelData_t::FromObjFile("models/colored_cube.obj")};
		phys_cube2.Transform.Position = glm::vec3{-4.0f, 0.0f, 0.0f};
		phys_cube2.Transform.Scale = glm::vec3{0.2, 0.2f, 0.2f};
		phys_cube2.TestingInterpolateType = 1;

		auto last_update_time = std::chrono::steady_clock::now();

		m_EntityServer.Start();


		Debug()->GetConsole()->LogInfo(MESSAGE_NO_FILE_LOG_BIT, "Secret shhhhhh.");
		while (!m_RenderingServer.WindowShouldClose()) {

			phys_cube.AddForce(glm::vec3{5.0f, 0.0f, 0.0f});
			phys_cube2.AddForce(glm::vec3{5.0f, 0.0f, 0.0f});

			Debug()->Profiler()->Begin("Main Loop");

			m_RenderingServer.PollEvents();
			m_InputServer.UpdateInput();

			if(m_InputServer.GetKeyDown(KEY_M)) {
				m_ShowMainUIWindow = true;
			}
			DrawMainUIWindow();

			if(m_InputServer.GetKeyUp(KEY_R)) {
				phys_cube.Stop();
				phys_cube2.Stop();
				phys_cube.Transform.Position = glm::vec3{-4.0f, 2.0f, 0.0f};
				phys_cube2.Transform.Position = glm::vec3{-4.0f, 0.0f, 0.0f};
			}
			if(m_InputServer.GetKeyUp(KEY_T)) {
				phys_cube.Transform.Position = glm::vec3{-4.0f, 2.0f, 0.0f};
				phys_cube2.Transform.Position = glm::vec3{-4.0f, 0.0f, 0.0f};
			}
			if(m_InputServer.GetKey(KEY_L)) {
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
			}

			static int i = 0;
			i++;

			auto current_time = std::chrono::steady_clock::now();

			std::chrono::duration<float> delta_time = current_time - last_update_time;
			delta_time = std::chrono::duration<float>{glm::min(delta_time.count(), MAX_FRAME_TIME)};
			delta_time = std::chrono::duration_cast<std::chrono::microseconds>(delta_time);

			last_update_time = current_time;

			m_RenderingServer.DrawLine(glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{1.0f, 0.0f, 0.0f}, glm::vec3{1.0f, 0.0f, 0.0f}, UNIQUE_NAME);
			m_RenderingServer.DrawLine(glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 1.0f, 0.0f}, glm::vec3{0.0f, 1.0f, 0.0f}, UNIQUE_NAME);
			m_RenderingServer.DrawLine(glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 0.0f, 1.0f}, glm::vec3{0.0f, 0.0f, 1.0f}, UNIQUE_NAME);

			m_EntityServer.Tick(delta_time.count() * 10e-1f); // For some reason, it seems that to get second we need
															  //  to multiply by 10e-1f and not the expected 10e-6f !
															  //  Related to issue #2

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