#include "rendering_server.h"
#include "../error_macros.h"
#include "../entities/lights/light.h"

#include "application.h"

#include "../core_macros.h"
#if USE_IMGUI
	#include "gui.h"
#endif

namespace gigno {

	RenderingServer::RenderingServer(int winw, int winh, const char *winTitle, InputServer *inputServer, const std::string &vertShaderFilePath, const std::string &fragShaderFilePath) :
		m_VertShaderFilePath{vertShaderFilePath},
		m_FragShaderFilePath{fragShaderFilePath},
		m_Window{ winw, winh, winTitle, inputServer },
		m_Device{&m_Window},
		m_SwapChain{ m_Device, &m_Window, vertShaderFilePath, fragShaderFilePath }
	{
		CreateSyncObjects();

#if USE_IMGUI
		GLFWwindow *glfwWindow = m_Window.GetGLFWwindow();
		if(glfwWindow){
			InitImGui(glfwWindow, m_Device, m_SwapChain);
		}
#endif
	}

	RenderingServer::~RenderingServer() {

#if USE_IMGUI
		ShutdownImGui();
#endif

		m_SwapChain.CleanUp(m_Device.GetDevice());

		for(rsize_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(m_Device.GetDevice(), m_ImageAvaliableSemaphores[i], nullptr);
			vkDestroySemaphore(m_Device.GetDevice(), m_RenderFinishedSemaphores[i], nullptr);
			vkDestroyFence(m_Device.GetDevice(), m_InFlightFences[i], nullptr);
		}
	}

	void RenderingServer::Finalize() {
		vkDeviceWaitIdle(m_Device.GetDevice());
	}

	void RenderingServer::PollEvents() {
		m_Window.PollEvents();
	}

	void RenderingServer::SubscribeRenderedEntity(RenderedEntity *entity) {
		entity->pNextRenderedEntity = m_pFirstRenderedEntity;
		m_pFirstRenderedEntity = entity;
	}

	void RenderingServer::UnsubscribeRenderedEntity(RenderedEntity *entity) {
		RenderedEntity *curr = m_pFirstRenderedEntity;
		if(curr == entity) {
			m_pFirstRenderedEntity = curr->pNextRenderedEntity;
			return;
		}
		while(curr) {
			if(curr->pNextRenderedEntity == entity) {
				curr->pNextRenderedEntity = entity->pNextRenderedEntity;
				return;
			}
			curr = curr->pNextRenderedEntity;
		}
		ERR_MSG("Tried to unsubsribe rendered entity '%s' but it was not subscribed.", entity->Name == "" ? "No name" : entity->Name.c_str());
	}

	void RenderingServer::SubscribeLightEntity(Light *light)
	{
		m_LightEntities.push_back(light);
	}

	void RenderingServer::UnsubscribeLightEntity(Light *light)
	{
		m_LightEntities.erase(std::remove(m_LightEntities.begin(), m_LightEntities.end(), light), m_LightEntities.end());
	}

	void RenderingServer::CreateModel(std::shared_ptr<giModel> &model, const ModelData_t &modelData) {
		model = std::make_shared<giModel>(giModel{ m_Device, modelData, m_SwapChain.GetCommandPool() });
	}

	//Debug Drawing
	void RenderingServer::DrawPoint(glm::vec3 pos, glm::vec3 color, const std::string &uniqueName) {
#if USE_DEBUG_DRAWING
		if(!ShowDD || !ShowDDPoints) { return; }
		m_SwapChain.DrawPoint(pos, color, std::hash<std::string>{}(uniqueName));
#endif
	}
	void RenderingServer::DrawLine(glm::vec3 startPos, glm::vec3 endPos, glm::vec3 color, const std::string &uniqueName) {
#if USE_DEBUG_DRAWING
		if (!ShowDD || !ShowDDLines) { return; }
		m_SwapChain.DrawLine(startPos, endPos, color, color, std::hash<std::string>{}(uniqueName));
#endif
	}
	void RenderingServer::DrawLineGradient(glm::vec3 startPos, glm::vec3 endPos, glm::vec3 startColor, glm::vec3 endColor, const std::string &uniqueName) {
#if USE_DEBUG_DRAWING
		if (!ShowDD || !ShowDDLines) { return; }
		m_SwapChain.DrawLine(startPos, endPos, startColor, endColor, std::hash<std::string>{}(uniqueName));
#endif
	}

	void RenderingServer::CreateSyncObjects() {
		m_ImageAvaliableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semCreateInfo{};
		semCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceCreateInfo{};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for(rsize_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			if (vkCreateSemaphore(m_Device.GetDevice(), &semCreateInfo, nullptr, &m_ImageAvaliableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(m_Device.GetDevice(), &semCreateInfo, nullptr, &m_RenderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(m_Device.GetDevice(), &fenceCreateInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS) {
				ERR_MSG("Failed to create Vulkan Sync Object Semaphore / Fence.");
			}
		}
	}


	void RenderingServer::Render() {
		DrawFrame();
	}

	void RenderingServer::DrawFrame() {
		vkWaitForFences(m_Device.GetDevice(), 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

		uint32_t imageIndex = 0;
		VkResult result = vkAcquireNextImageKHR(m_Device.GetDevice(), m_SwapChain.GetSwapChain(), UINT64_MAX, m_ImageAvaliableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_Window.HasResized()) {
			m_SwapChain.Recreate(m_Device, &m_Window, m_VertShaderFilePath, m_FragShaderFilePath);
			return;
		}
		else if (result != VK_SUCCESS) {
			ERR_MSG("Failed to Acquire Swap Chain Image ! Vulkan Error Code : %d", (int) result);
		}

		vkResetFences(m_Device.GetDevice(), 1, &m_InFlightFences[m_CurrentFrame]);

		vkResetCommandBuffer(m_SwapChain.GetCommandBuffer(m_CurrentFrame), 0);

		#if USE_DEBUG_DRAWING
		m_SwapChain.UpdateDebugDrawings(m_Device.GetDevice(), m_Device.GetPhysicalDevice(), m_Device.GetGraphicsQueue());
		#endif

		SceneRenderingData_t sceneData{m_pFirstRenderedEntity, m_LightEntities, m_pCamera};
		m_SwapChain.RecordCommandBuffer(m_CurrentFrame, imageIndex, sceneData);

		VkSemaphore waitSemaphores[] = { m_ImageAvaliableSemaphores[m_CurrentFrame]};
		VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphores[m_CurrentFrame]};
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = m_SwapChain.GetCommandBufferPtr(m_CurrentFrame);
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(m_Device.GetDevice(), 1, &m_InFlightFences[m_CurrentFrame]);

		result = vkQueueSubmit(m_Device.GetGraphicsQueue(), 1, &submitInfo, m_InFlightFences[m_CurrentFrame]);
		if (result != VK_SUCCESS) {
			ERR_MSG("Failed to Submit to Graphics Queue ! Vulkan Error Code : %d", (int)result);
		}

		VkSwapchainKHR swapchains[] = { m_SwapChain.GetSwapChain() };

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapchains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr;

		result = vkQueuePresentKHR(m_Device.GetPresentQueue(), &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			m_SwapChain.Recreate(m_Device, &m_Window, m_VertShaderFilePath, m_FragShaderFilePath);
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			ERR_MSG("Failed to Present Swap Chain Image ! Vulkan Error Code : %d", (int)result);
		} 

		m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

	#if USE_IMGUI
		NewFrameImGui();
	#endif
	}
}