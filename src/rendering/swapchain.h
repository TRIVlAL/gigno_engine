#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "vulkan/vulkan.h"
#include "pipeline.h"
#include <vector>
#include <memory>
#include <string>

#include "glm/glm.hpp"

namespace gigno {

	const int MAX_FRAMES_IN_FLIGHT = 2;

	struct PushConstantData_t {
		glm::mat4 modelMatrix;
		glm::mat3 normalsMatrix;
	};

	struct UniformBufferData_t {
		glm::mat4 view{ 1.f };
		glm::mat4 projection{ 1.f };
		//glm::mat3 normals{ 1.f };
	};

	class giDevice;
	struct SwapChainSupportDetails;
	struct QueueFamilyIndices;
	class giWindow;
	class giPipeline;
	class giModel;
	class RenderedEntity;
	class Camera;

	class giSwapChain
	{
	public:
		giSwapChain(const giDevice &device, const giWindow *window, const std::string &vertShaderPath, const std::string &fragShaderPath);

		void CleanUp(VkDevice device);

		void Recreate(const giDevice &device, const giWindow *window, const std::string &vertShaderPath, const std::string &fragShaderPath);

		uint32_t GetWidth() const { return m_Extent.width; }
		uint32_t GetHeight() const { return m_Extent.height; }
		VkRenderPass GetRenderPass() const { return m_RenderPass; }
		VkSwapchainKHR GetSwapChain() const { return m_VkSwapChain; }
		VkCommandBuffer GetCommandBuffer(uint32_t index) const { return m_CommandBuffers[index]; }
		VkCommandBuffer const *GetCommandBufferPtr(uint32_t index) const { return &m_CommandBuffers[index]; }
		VkCommandPool GetCommandPool() const { return m_CommandPool; }
		void RecordCommandBuffer(uint32_t currentFrame, uint32_t imageIndex, const std::vector<const RenderedEntity *> &renderedEntities, const Camera *camera);

	private:
		void CreateDescriptorSetLayout(VkDevice device); 
		void CreateDescriptorPool(VkDevice device);
		void CreateDescriptorSets(VkDevice device);
		void CreateUniformBuffers(VkDevice device, VkPhysicalDevice physDevice);
		void CreatePipelineLayout(VkDevice device);
		void CreatePipeline(VkDevice device, const std::string &vertShaderPath, const std::string &fragShaderPath);
		void CreateVkSwapChain(VkDevice device, const SwapChainSupportDetails &supportDetails, const QueueFamilyIndices &physicalDeviceQueueFamilyIndices, 
								VkSurfaceKHR surface, const giWindow *window, bool isFirstCreation);
		void CreateImageViews(VkDevice device);
		void CreateRenderPass(const VkDevice &device, const VkPhysicalDevice &physDevice);
		void CreateFrameBuffers(VkDevice device);
		void CreateCommandPool(VkDevice device, QueueFamilyIndices queueFamilyIndices);
		void CreateDepthResources(VkDevice device, VkPhysicalDevice physDevice);
		void CreateCommandBuffers(VkDevice device);

		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &avaliableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &avaliablePresentModes);
		VkExtent2D ChooseSwapExtent(const giWindow *window, const VkSurfaceCapabilitiesKHR &capabilities);
		VkFormat FindSupportedFormat(VkPhysicalDevice physDevice, const std::vector<VkFormat> &candidates, VkImageTiling targetTiling, VkFormatFeatureFlags targetFeatures);
		VkFormat FindDepthFormat(VkPhysicalDevice physDevice);
		uint32_t FindMemoryTypeIndex(VkPhysicalDevice device, uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
		void CreateImage(VkDevice device, VkPhysicalDevice physDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, 
							VkMemoryPropertyFlags props, VkImage &image, VkDeviceMemory &imageMemory);
		VkImageView CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

		void RecordCommandBuffer(VkCommandBuffer buffer, uint32_t currentFrame, uint32_t imageIndex, const std::vector<const RenderedEntity *> &renderedEntities, const Camera *camera);

		void RenderEntities(VkCommandBuffer buffer, const std::vector<const RenderedEntity *> &entities, uint32_t currentFrame);
		void EntitiesUpdateUniformBuffer(const std::vector<const RenderedEntity *> &entities, uint32_t currentFrame);

		void UpdateUniformBuffer(VkCommandBuffer commandBuffer, const Camera *camera, uint32_t currentFrame);

		VkFormat m_Format;
		VkExtent2D m_Extent;
		VkRenderPass m_RenderPass;

		std::vector<VkImage> m_Images;
		std::vector<VkImageView> m_ImageViews;
		std::vector<VkFramebuffer> m_FrameBuffers;

		VkImage m_DepthImage;
		VkDeviceMemory m_DepthImageMemory;
		VkImageView m_DepthImageView;

		VkCommandPool m_CommandPool;
		std::vector<VkCommandBuffer> m_CommandBuffers;

		VkDescriptorSetLayout m_DescriptorSetLayout;
		VkDescriptorPool m_DescriptorPool;
		std::vector<VkDescriptorSet> m_DescriptorSets;

		std::vector<VkBuffer> m_UniformBuffers;
		std::vector<VkDeviceMemory> m_UniformBuffersMemories;
		std::vector<void *> m_UniformBuffersMapped;


		std::unique_ptr<giPipeline> m_Pipeline;
		VkPipelineLayout m_PipelineLayout;

		VkSwapchainKHR m_VkSwapChain;


	};

}

#endif