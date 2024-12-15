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
#include "entities/keyvalues/key_table.h"
#include "rendering/gui.h"

namespace gigno {

	Application *Application::MakeApp() {
		Application *app = new Application(1000, 1000, "Gigno Engine Demo", "shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv");
		return app;
	}
	
	void Application::ShutdownApp() {
		ASSERT_MSG(s_Instance, "MakeApp() must be called before ShutdownApp() !");
		delete(s_Instance);
	}

	Application::Application(int winw, int winh, const char *title, const std::string &vertShaderPath, const std::string &fragShaderPath) :
		m_DebugServer{},
		m_InputServer{},
		m_RenderingServer{ winw, winh, title, &m_InputServer, vertShaderPath, fragShaderPath },
		m_EntityServer{},
		m_PhysicServer{} {
			if(s_Instance) {
				ERR_MSG("Multiple applications created !");
			}
			s_Instance = this;
		}

	Application::~Application() {
		if(s_Instance == this) {
			s_Instance = nullptr;
		}
	}

	Application *Application::Singleton() {
		return s_Instance;
	}

	int Application::run() {
		Console::StartFileLogging ();

		ASSERT_MSG_V(glfwInit(), 1, "GLFW Failed to init");

		DomeCamera camera(17.0f);
		camera.SetPerspectiveProjection(glm::radians(50.0f), m_RenderingServer.GetAspectRatio(), -0.05f, 1.0f);
		camera.Position = { 0.0f, 0.0f, -17.0f };
		camera.Rotation.y = 0;
		camera.MaxLower = 0.0f;
		camera.SetTarget( glm::vec3{0.0f} );
		camera.Name = "My Camera";


		DirectionalLight sun;
		sun.Intensity = 0.4f;
		sun.Direction = glm::normalize(glm::vec3{0.0f, -1.0f, 0.01f});

		PointLight bulb2;
		bulb2.Position = glm::vec3{10.0f, 1.0f, 10.0f};
		bulb2.Intensity = 2.0f;

		PointLight bulb3;
		bulb3.Position = glm::vec3{-10.0f, 1.0f, 10.0f};
		bulb3.Intensity = 2.0f;

		PointLight bulb4;
		bulb4.Position = glm::vec3{10.0f, 1.0f, -10.0f};
		bulb4.Intensity = 2.0f;

		PointLight bulb5;
		bulb5.Position = glm::vec3{-10.0f, 1.0f, -10.0f};
		bulb5.Intensity = 2.0f;

		EnvironmentLight env;
		env.intensity = 0.1f;

		/*
		RigidBody phys_capsule{ModelData_t::FromObjFile("models/capsule.obj")};
		phys_capsule.Position = glm::vec3{-1.0f, 6.5f, 3.0f};
		phys_capsule.Rotation = glm::vec3{0.0f, 0.0f, 0.0f};
		phys_capsule.Scale = glm::vec3{0.5, 0.5f, 0.5f};
		phys_capsule.GiveCapsuleCollider(0.5f, 1.5f);
		phys_capsule.AddImpulse(glm::vec3{0.0f, -10.4f, -6.25f});
		phys_capsule.Mass = 20.0f;
		phys_capsule.Material = MAT_PLASTIC;
		*/

		RigidBody phys_sphere{};
		phys_sphere.ModelPath = "models/colored_uv_sphere.obj";
		phys_sphere.Position = glm::vec3{0.3f, 2.5f, 4.0f};
		phys_sphere.Scale = glm::vec3{0.5, 0.5f, 0.5f};
		phys_sphere.GiveSphereCollider(0.5f);
		phys_sphere.AddImpulse(glm::vec3{0.0f, -10.4f, -6.25f});
		phys_sphere.Mass = 50.0f;
		phys_sphere.Material = MAT_STEEL;

		RigidBody phys_sphere2{};
		phys_sphere2.ModelPath = "models/uv_sphere.obj";
		phys_sphere2.Position = glm::vec3{-0.5f, -0.5f, -1.0f};
		phys_sphere2.Scale = glm::vec3{0.5, 0.5f, 0.5f};
		phys_sphere2.GiveSphereCollider(0.5f);
		phys_sphere2.AddImpulse(glm::vec3{0.0f, 3.0f, 6.0f});
		phys_sphere2.Mass = 10.0f;
		phys_sphere2.Material = MAT_PLASTIC;

		RigidBody phys_sphere3{};
		phys_sphere3.ModelPath = "models/uv_sphere.obj";
		phys_sphere3.Position = glm::vec3{-3.0f, 2.5f, 0.0f};
		phys_sphere3.Scale = glm::vec3{0.5, 0.5f, 0.5f};
		phys_sphere3.GiveSphereCollider(0.5f);
		phys_sphere3.AddImpulse(glm::vec3{6.0f, -6.0f, 1.0f});
		phys_sphere3.Mass = 10.0f;
		phys_sphere3.Material = MAT_PLASTIC;
		

		RigidBody phys_plane{};
		phys_plane.ModelPath = "models/plane_subdivided.obj";
		phys_plane.Position = glm::vec3{0.0f, -1.0f, 0.0f};
		phys_plane.Scale = glm::vec3{2.0f};
		phys_plane.GivePlaneCollider(glm::vec3{0.0f, 1.0f, 0.0f});
		phys_plane.IsStatic = true;
		phys_plane.Material = MAT_CONCRETE;

		RigidBody phys_plane2{};
		phys_plane2.ModelPath = "models/plane_subdivided.obj";
		phys_plane2.Position = glm::vec3{0.0f, 19.0f, -20.0f};
		phys_plane2.Rotation = glm::vec3{glm::pi<float>() / 2.0f, 0.0f, 0.0f};
		phys_plane2.Scale = glm::vec3{2.0f};
		phys_plane2.GivePlaneCollider(glm::vec3{0.0f, 0.0f, 1.0f});
		phys_plane2.IsStatic = true;
		phys_plane2.Material = MAT_CONCRETE;

		RigidBody phys_plane3{};
		phys_plane3.ModelPath = "models/plane_subdivided.obj";
		phys_plane3.Position = glm::vec3{0.0f, 19.0f, 20.0f};
		phys_plane3.Rotation = glm::vec3{-glm::pi<float>() / 2.0f, 0.0f, 0.0f};
		phys_plane3.Scale = glm::vec3{2.0f};
		phys_plane3.GivePlaneCollider(glm::vec3{0.0f, 0.0f, -1.0f});
		phys_plane3.IsStatic = true;
		phys_plane3.Material = MAT_CONCRETE;

		RigidBody phys_plane4{};
		phys_plane4.ModelPath = "models/plane_subdivided.obj";
		phys_plane4.Position = glm::vec3{ -20.0f, 19.0f, 0.0f};
		phys_plane4.Rotation = glm::vec3{0.0f, 0.0f, glm::pi<float>() / 2.0f};
		phys_plane4.Scale = glm::vec3{2.0f};
		phys_plane4.GivePlaneCollider(glm::vec3{1.0f, 0.0f, 0.0f});
		phys_plane4.IsStatic = true;
		phys_plane4.Material = MAT_CONCRETE;

		RigidBody phys_plane5{};
		phys_plane5.ModelPath = "models/plane_subdivided.obj";
		phys_plane5.Position = glm::vec3{20.0f, 19.0f, 0.0f};
		phys_plane5.Rotation = glm::vec3{0.0f, 0.0f, -glm::pi<float>() / 2.0f};
		phys_plane5.Scale = glm::vec3{2.0f};
		phys_plane5.GivePlaneCollider(glm::vec3{-1.0f, 0.0f, 0.0f});
		phys_plane5.IsStatic = true;
		phys_plane5.Material = MAT_CONCRETE;

		Spinner spinner{};
		spinner.ModelPath = "models/colored_cube.obj";
		spinner.Speed = 2.0f;

		//TestEntity test{&phys_sphere, &phys_sphere2};

		auto last_update_time = std::chrono::steady_clock::now();

		m_EntityServer.Start();

		auto start_time = std::chrono::steady_clock::now();

		Work();

		Console::LogInfo (MESSAGE_NO_FILE_LOG_BIT, "Secret shhhhhh.");
		while (!m_RenderingServer.WindowShouldClose() && !Close) {
			Profiler::Begin("Main Loop");

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

			Profiler::End(); //Main Loop

			Debug()->Update();
		}

		return 0;
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