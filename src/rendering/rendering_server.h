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
	class InputServer;

	struct SceneRenderingData_t {
		const RenderedEntity * RenderedEntities; // First entity in the chain.
		const std::vector<const Light *> &LightEntities;
		const Camera *pCamera;
	};

	class RenderingServer {

	public:
		RenderingServer(int winw, int winh, const char *winTitle, InputServer *inputServer, const std::string &vertShaderFilePath, const std::string &fragShaderFilePath);
		~RenderingServer();

		bool WindowShouldClose() { return m_Window.ShouldClose(); }
		void PollEvents();

		void Render();

		void SubscribeRenderedEntity(RenderedEntity *entity);
		void UnsubscribeRenderedEntity(RenderedEntity *entity);

		void SubscribeLightEntity(Light *light);
		void UnsubscribeLightEntity(Light *light);

		void SetCurrentCamera(const Camera *camera) { m_pCamera = camera; }
		bool HasCamera() const { return m_pCamera != nullptr; }
		const Camera *GetCameraHandle() const { return m_pCamera; }

		float GetAspectRatio() { return static_cast<float>(m_SwapChain.GetWidth()) / static_cast<float>(m_SwapChain.GetHeight()); }

		void CreateModel(std::shared_ptr<giModel> &model, const ModelData_t &modelData);

		//Debug Drawing ( need to active USE_DEBUG_DRAWING in features_usage.h )
		void DrawPoint(glm::vec3 pos, glm::vec3 color, const std::string &uniqueName );
		void DrawLine(glm::vec3 startPos, glm::vec3 endPos, glm::vec3 color, const std::string &uniqueName);
		void DrawLineGradient(glm::vec3 startPos, glm::vec3 endPos, glm::vec3 startColor, glm::vec3 endColor, const std::string &uniqueName);

		#if USE_DEBUG_DRAWING
		bool ShowDD = true;
		bool ShowDDPoints = true;
		bool ShowDDLines = true;
		#endif

	private:
		void CreateSyncObjects();

		void DrawFrame();

		uint32_t m_CurrentFrame = 0;

		Window m_Window;
		Device m_Device;
		SwapChain m_SwapChain;

		// First rendered entity in the chain of all rendered entity (linked list). Use entity->pNextRenderedEntity for next element in the list.
		// If this is null, there are no rendered entity. If next is null, it is the last rendered entity.
		RenderedEntity *m_pFirstRenderedEntity{};

		std::vector<const Light *> m_LightEntities;

		const Camera *m_pCamera = nullptr;

		std::vector<VkSemaphore> m_ImageAvaliableSemaphores;
		std::vector<VkSemaphore> m_RenderFinishedSemaphores;
		std::vector<VkFence> m_InFlightFences;

		std::string m_VertShaderFilePath;
		std::string m_FragShaderFilePath;

		float m_RenderTime = 0.0f;
	};

	#define STRINGIFY1(a) STRINGIFY2(a)
	#define STRINGIFY2(a) #a
	#define UNIQUE_NAME STRINGIFY1( __LINE__ ) __FILE__

}
#endif