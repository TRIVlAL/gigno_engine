#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "vulkan/vulkan.h"
#include "pipeline.h"
#include <vector>
#include <memory>
#include <string>

#include "model.h"

#include "../features_usage.h"

#include "glm/glm.hpp"

namespace gigno {

	const size_t MAX_FRAMES_IN_FLIGHT = 2;

	/*
	Per-Entity data pushed to the shader.
	
	Implementation :
		* Must follow the Vulkan alignment rules : (From vulkan-tutorial) https://vulkan-tutorial.com/Uniform_buffers/Descriptor_pool_and_sets#page_Alignment-requirements
	*/
	struct PushConstantData_t {
		glm::mat4 modelMatrix;
		glm::mat4 normalsMatrix;
		alignas(4) int fullbright = 0;
	};

	const int MAX_LIGHT_DATA_COUNT = 15;

	/*
	Constant-during-the-frame data pushed to the shader.

	Implementation :
		* Must follow the Vulkan alignment rules : (From vulkan-tutorial) https://vulkan-tutorial.com/Uniform_buffers/Descriptor_pool_and_sets#page_Alignment-requirements
	*/
	struct UniformBufferData_t {
		glm::mat4 view{ 1.f };
		glm::mat4 projection{ 1.f };
		glm::vec4 lightDatas[MAX_LIGHT_DATA_COUNT];
	};

	class Device;
	struct SwapChainSupportDetails;
	struct QueueFamilyIndices;
	struct SceneRenderingData_t;
	class Window;
	class giPipeline;
	class giModel;
	class RenderedEntity;
	class Light;
	class Camera;

	/*
	Creates/Holds/Destroys Vulkan structs necessary for rendering.
	
	Usage :

		* Initialisation : Initialized by constructor.
		* Clean Up : Must Call CleanUp() with the same device as the one used for construction as parameter.
		* Key Funcitons : 
		  * void RecordCommandBuffer( ... )
		  * void Recreate( ... )
		* Lifetime : Must outlive the device used for initialization/clean up.
	*/
	class SwapChain
	{
	public:
		SwapChain(const Device &device, const Window *window, const std::string &vertShaderPath, const std::string &fragShaderPath);

		void CleanUp(VkDevice device);

		uint32_t GetWidth() const { return m_Extent.width; }
		uint32_t GetHeight() const { return m_Extent.height; }
		VkRenderPass GetRenderPass() const { return m_RenderPass; }
		VkSwapchainKHR GetSwapChain() const { return m_VkSwapChain; }
		VkCommandBuffer GetCommandBuffer(uint32_t index) const { return m_CommandBuffers[index]; }
		VkCommandBuffer const *GetCommandBufferPtr(uint32_t index) const { return &m_CommandBuffers[index]; }
		VkCommandPool GetCommandPool() const { return m_CommandPool; }

		/* 
		@brief Recreates the Vulkan structs needed. To be called if the window size changed.
		@param device same device as the one used on initialization/clean up 
		*/
		void Recreate(const Device &device, const Window *window, const std::string &vertShaderPath, const std::string &fragShaderPath);

		/* 
		@brief Fills the command buffer so that it is ready to be Submitted to vulkan. Updates the data pushed to the shader (Push Constants, Uniform Buffer)
		@param currentFrame smaller than MAX_FRAMES_IN_FLIGHT 
		*/
		void RecordCommandBuffer(uint32_t currentFrame, uint32_t imageIndex, const SceneRenderingData_t &sceneData);

		#if USE_DEBUG_DRAWING
		void UpdateDebugDrawings(VkDevice device, VkPhysicalDevice physDevice, VkQueue graphicsQueue);

		void DrawPoint(glm::vec3 position, glm::vec3 color, uint32_t drawCallHash);
		void DrawLine(glm::vec3 startPos, glm::vec3 endPos, glm::vec3 startColor, glm::vec3 endColor, uint32_t drawCallHash);
		#endif
		
	private:
		void CreateDescriptorSetLayout(VkDevice device); 
		void CreateDescriptorPool(VkDevice device);
		void CreateDescriptorSets(VkDevice device);
		void CreateUniformBuffers(VkDevice device, VkPhysicalDevice physDevice);
		void CreatePipelineLayout(VkDevice device);
		void CreatePipeline(VkDevice device, const std::string &vertShaderPath, const std::string &fragShaderPath);
		void CreateVkSwapChain(VkDevice device, const SwapChainSupportDetails &supportDetails, const QueueFamilyIndices &physicalDeviceQueueFamilyIndices, 
								VkSurfaceKHR surface, const Window *window, bool isFirstCreation);
		void CreateImageViews(VkDevice device);
		void CreateRenderPass(const VkDevice &device, const VkPhysicalDevice &physDevice);
		void CreateFrameBuffers(VkDevice device);
		void CreateCommandPool(VkDevice device, QueueFamilyIndices queueFamilyIndices);
		void CreateDepthResources(VkDevice device, VkPhysicalDevice physDevice);
		void CreateCommandBuffers(VkDevice device);

		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &avaliableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &avaliablePresentModes);
		VkExtent2D ChooseSwapExtent(const Window *window, const VkSurfaceCapabilitiesKHR &capabilities);
		VkFormat FindSupportedFormat(VkPhysicalDevice physDevice, const std::vector<VkFormat> &candidates, VkImageTiling targetTiling, VkFormatFeatureFlags targetFeatures);
		VkFormat FindDepthFormat(VkPhysicalDevice physDevice);
		uint32_t FindMemoryTypeIndex(VkPhysicalDevice device, uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
		void CreateImage(VkDevice device, VkPhysicalDevice physDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, 
							VkMemoryPropertyFlags props, VkImage &image, VkDeviceMemory &imageMemory);
		VkImageView CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

		void RecordCommandBuffer(VkCommandBuffer buffer, uint32_t currentFrame, uint32_t imageIndex, const SceneRenderingData_t &sceneData);

		void RenderEntities(VkCommandBuffer buffer, const RenderedEntity * entities, uint32_t currentFrame);

		#if USE_DEBUG_DRAWING
		void RenderDebugDrawings(VkCommandBuffer buffer, const Camera *camera, uint32_t currentFrame);
		#endif

		void UpdateUniformBuffer(VkCommandBuffer commandBuffer, const Camera *camera, const std::vector<const Light *> &lights, uint32_t currentFrame);

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

		#if USE_DEBUG_DRAWING
		uint32_t m_DDLastFramePointsDrawCallHash;
		uint32_t m_DDCurrentFramePointsDrawCallHash;

		uint32_t m_DDLastFrameLinesDrawCallHash;
		uint32_t m_DDCurrentFrameLinesDrawCallHash;

		uint32_t m_DDPointsCount = 0;
		std::vector<Vertex> m_DDPoints { };

		uint32_t m_DDLinesCount = 0;
		std::vector<Vertex> m_DDLines{ };

		VkBuffer m_DDPointsBuffer;
		VkDeviceMemory m_DDPointsMemory;

		VkBuffer m_DDLinesBuffer;
		VkDeviceMemory m_DDLinesMemory;
		#endif
	};

}

#endif