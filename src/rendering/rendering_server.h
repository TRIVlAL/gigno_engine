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
	class giInputServer;

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

		void SetCurrentCamera(const Camera *camera) { m_pCamera = camera; }
		bool HasCamera() const { return m_pCamera != nullptr; }

		float GetAspectRatio() { return static_cast<float>(m_SwapChain.GetWidth()) / static_cast<float>(m_SwapChain.GetHeight()); }

		void CreateModel(std::shared_ptr<giModel> &model, const ModelData_t &modelData);

	private:
		void CreateSyncObjects();

		void DrawFrame();

		uint32_t m_CurrentFrame = 0;

		giWindow m_Window;
		giDevice m_Device;
		giSwapChain m_SwapChain;
		
		std::vector<const RenderedEntity *> m_RenderedEntities;

		const Camera *m_pCamera = nullptr;

		std::vector<VkSemaphore> m_ImageAvaliableSemaphores;
		std::vector<VkSemaphore> m_RenderFinishedSemaphores;
		std::vector<VkFence> m_InFlightFences;

		std::string m_VertShaderFilePath;
		std::string m_FragShaderFilePath;

		bool m_WasLastRenderAborted = false;
	};

}

#endif