#ifndef RENDERING_SERVER_H
#define RENDERING_SERVER_H

#include "window.h"
#include "device.h"

#include "../entities/camera.h"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"

#include <memory>

#include "../entities/rendered_entity.h"

namespace gigno {
	class Light;
	class giInputServer;

	struct SceneRenderingData_t {
		const std::vector<const RenderedEntity *> &RenderedEntities;
		const std::vector<const Light *> &LightEntities;
		const Camera *pCamera;
	};

	class giRenderingServer {

	public:
		giRenderingServer(int winw, int winh, const char *winTitle, giInputServer *inputServer, const std::string &vertShaderFilePath, const std::string &fragShaderFilePath);
		~giRenderingServer();

		void Finalize();

		bool WindowShouldClose() { return m_Window.ShouldClose(); }
		void PollEvents();

		void Render();

		void SubscribeRenderedEntity(RenderedEntity *entity);
		void UnsubscribeRenderedEntity(RenderedEntity *entity);

		void SubscribeLightEntity(Light *light);
		void UnsubscribeLightEntity(Light *light);

		void SetCurrentCamera(const Camera *camera) { m_pCamera = camera; }
		bool HasCamera() const { return m_pCamera != nullptr; }

		float GetAspectRatio() { return static_cast<float>(m_SwapChain.GetWidth()) / static_cast<float>(m_SwapChain.GetHeight()); }

		void CreateModel(std::shared_ptr<giModel> &model, const ModelData_t &modelData);

		//Debug Drawing ( need to active USE_DEBUG_DRAWING in core_macros.h )
		void DrawPoint(glm::vec3 pos, glm::vec3 color, const std::string &uniqueName );
		void DrawLine(glm::vec3 startPos, glm::vec3 endPos, glm::vec3 color, const std::string &uniqueName);
		void DrawLineGradient(glm::vec3 startPos, glm::vec3 endPos, glm::vec3 startColor, glm::vec3 endColor, const std::string &uniqueName);

		#if USE_IMGUI
		void OpenDebugWindow() { m_ShowDebugWindow = true; }
		void CloseDebugWindow() { m_ShowDebugWindow = false; }
		void ToggleDebugWindow() { m_ShowDebugWindow = !m_ShowDebugWindow; }
		#endif

	private:
		void CreateSyncObjects();

		void DrawFrame();

		uint32_t m_CurrentFrame = 0;

		giWindow m_Window;
		giDevice m_Device;
		giSwapChain m_SwapChain;
		
		std::vector<const RenderedEntity *> m_RenderedEntities;
		std::vector<const Light *> m_LightEntities;

		const Camera *m_pCamera = nullptr;

		std::vector<VkSemaphore> m_ImageAvaliableSemaphores;
		std::vector<VkSemaphore> m_RenderFinishedSemaphores;
		std::vector<VkFence> m_InFlightFences;

		std::string m_VertShaderFilePath;
		std::string m_FragShaderFilePath;

		bool m_WasLastRenderAborted = false;

		float m_RenderTime = 0.0f;

		#if USE_IMGUI
		bool m_ShowDebugWindow = false;
		void ShowDebugWindow();
		#endif

		#if USE_DEBUG_DRAWING
		bool m_ShowDD = true;
		bool m_ShowDDPoints = true;
		bool m_ShowDDLines = true;
		#endif
	};

	#define STRINGIFY1(a) STRINGIFY2(a)
	#define STRINGIFY2(a) #a
	#define UNIQUE_NAME STRINGIFY1( __LINE__ ) __FILE__

}
#endif