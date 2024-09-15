#include "application.h"
#include "error_macros.h"
#include <numeric>
#include <chrono>
#include "entities/dome_camera.h"
#include "entities/spinner.h"

namespace gigno {

	giApplication::giApplication(int winw, int winh, const char *title, const std::string &vertShaderPath, const std::string &fragShaderPath) :
		m_InputServer{},
		m_RenderingServer{ winw, winh, title, &m_InputServer, vertShaderPath, fragShaderPath },
		m_EntityServer{} {}

	giApplication::~giApplication() {}

	giEntityServer *giApplication::GetEntityServer() {
		return &m_EntityServer;
	}

	giApplication *giApplication::MakeApp() {
		giApplication *app = new giApplication(1000, 1000, "Gigno Engine Demo", "shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv");
		std::cout << "Created App !" << std::endl;
		s_Instance = app;
		return app;
	}

	giApplication *giApplication::Singleton() {
		ASSERT_MSG_V(s_Instance, "Calling giApplication::Singleton but s_Instance has not been assigned !", nullptr);
		return s_Instance;
	}

	int giApplication::run() {
		if (!glfwInit()) {
			return 1;
		}

		RenderedEntity first{ModelData_t::FromObjFile("models/smooth_vase.obj")};
		first.Transform.translation = glm::vec3{ 0.0f, 0.0f, 0.0f };
		first.Transform.scale = glm::vec3{ 3.0f, -3.0f, 3.0f };
		first.Transform.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
		float rot = 0;
		float y = -5.0f;

		Spinner second{ModelData_t::FromObjFile("models/colored_cube.obj")};
		second.Transform.translation = glm::vec3{ 2.0f, 0.5f, 1.0f };
		second.Transform.scale = glm::vec3{0.5f, 0.5f, 0.5f};
		second.Transform.rotation = glm::vec3(0.0f, glm::radians(30.0f), 0.0f);
		second.Speed = 1e10;

		Spinner third{ModelData_t::FromObjFile("models/colored_cube.obj")};
		third.Transform.translation = glm::vec3{-1.5f, 0.75f, -0.5f};
		third.Transform.scale = glm::vec3{0.25f, 0.5f, 0.25f};
		third.Transform.rotation = glm::vec3(0.0f, glm::radians(55.0f), 0.0f);
		third.Speed = -2e10f;

		DomeCamera camera(1000000000.0f);
		camera.SetPerspectiveProjection(glm::radians(50.0f), m_RenderingServer.GetAspectRatio(), -0.05f, 1.0f);
		camera.Transform.translation = { 0.0f, 0.0f, -6.0f };
		camera.Transform.rotation.y = 0;
		camera.SetTarget( (first.Transform.translation + second.Transform.translation + third.Transform.translation) * 0.5f );

		auto lastUpdateTime = std::chrono::steady_clock::now();
		double aggreg = 0.0;

		while (!m_RenderingServer.WindowShouldClose()) {
			m_RenderingServer.WindowPollEvents();

			auto currentTime = std::chrono::steady_clock::now();
			std::chrono::duration<double> deltaTime = currentTime - lastUpdateTime;
			deltaTime = std::chrono::duration<double>{ glm::min(deltaTime.count(), MAX_FRAME_TIME) };
			std::cout << "before " << deltaTime.count() << std::endl;
			deltaTime = std::chrono::duration_cast<std::chrono::nanoseconds>(deltaTime);
			std::cout << "after " << deltaTime.count() << std::endl;
			lastUpdateTime = currentTime;
			std::cout << "convert " << deltaTime.count() * 10e-9 << std::endl;
			
			m_EntityServer.Tick(deltaTime.count() * 10e-9);

			aggreg += deltaTime.count();

			m_RenderingServer.Render();
		}

		std::cout << "aggreg : " << aggreg << std::endl;

		m_RenderingServer.Finalize();

		return 0;
	}

	void giApplication::ShutdownApp() {
		giApplication *app = s_Instance;
		s_Instance = nullptr;
		delete(app);
	}
}