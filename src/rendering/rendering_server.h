#ifndef RENDERING_SERVER_H
#define RENDERING_SERVER_H

#include "window.h"
#include "device.h"

#include "../entities/camera.h"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"

#include <memory>

#include "../entities/rendered_entity.h"
#include "pipeline.h"

namespace gigno {
	class Light;
	class InputServer;

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

	struct SceneRenderingData_t {
		RenderedEntity * RenderedEntities; // First entity in the chain.
		Light *LightEntities; // First light in the chain.
		const Camera *pCamera;
	};


	class RenderingServer {

	public:
		RenderingServer() = default;
		~RenderingServer();

		void Init(int winw, int winh, const char *winTitle);

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

		float GetAspectRatio() { return static_cast<float>(m_SwapChain.Extent.width) / static_cast<float>(m_SwapChain.Extent.height); }

		void CreateModel(std::shared_ptr<giModel> &model, const ModelData_t &modelData);
		void ClenUpModel(std::shared_ptr<giModel> &model);

		//Debug Drawing ( need to active USE_DEBUG_DRAWING in features_usage.h )
		void DrawPoint(glm::vec3 pos, glm::vec3 color);
		void DrawLine(glm::vec3 startPos, glm::vec3 endPos, glm::vec3 color);
		void DrawLineGradient(glm::vec3 startPos, glm::vec3 endPos, glm::vec3 startColor, glm::vec3 endColor);

		#if USE_DEBUG_DRAWING
		bool ShowDD = true;
		bool ShowDDPoints = true;
		bool ShowDDLines = true;
		#endif

	private:
		void CreateSyncObjects();
		
		void CreateVkSwapChain(bool isFirstCreation);
		void CreateImageViews();
		void CreateRenderPass();
		void CreateDescriptorSetLayout();
		void CreatePipelineLayout();
		void CreatePipeline();
		void CreateCommandPool();
		void CreateDepthResources();
		void CreateFrameBuffers();
		void CreateUniformBuffers();
		void CreateDescriptorPool();
		void CreateDescriptorSets();
		void CreateCommandBuffers();

		void Recreate();

		void DrawFrame();

		void RecordCommandBuffer(SceneRenderingData_t &sceneData, uint32_t imageIndex);
		void UpdateUniformBuffer(const Camera *camera, Light *lights);
		void RenderEntities(RenderedEntity *entities);
		#if USE_DEBUG_DRAWING
		void RenderDebugDrawings(const Camera *camera);

		void UpdateDebugDrawings();
		#endif

		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &avaliableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &avaliablePresentModes);
		VkExtent2D ChooseSwapExtent(const Window *window, const VkSurfaceCapabilitiesKHR &capabilities);
		VkFormat FindSupportedFormat(VkPhysicalDevice physDevice, const std::vector<VkFormat> &candidates, VkImageTiling targetTiling, VkFormatFeatureFlags targetFeatures);
		VkFormat FindDepthFormat(VkPhysicalDevice physDevice);
		uint32_t FindMemoryTypeIndex(VkPhysicalDevice device, uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
		void CreateImage(VkDevice device, VkPhysicalDevice physDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
						 VkMemoryPropertyFlags props, VkImage &image, VkDeviceMemory &imageMemory);
		VkImageView CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

		uint32_t m_CurrentFrame = 0;

		//------------------------------------------------------

		Window m_Window;
		Device m_Device;

		//SwapChain m_SwapChain;

		struct {
			VkSwapchainKHR SwapChain;
			std::vector<VkImage> Images;
			VkFormat Format;
			VkExtent2D Extent;
			std::vector<VkImageView> ImageViews;
			std::vector<VkFramebuffer> Framebuffers;
		} m_SwapChain;

		VkRenderPass m_RenderPass;
		VkDescriptorSetLayout m_DescriptorSetLayout;
		VkPipelineLayout m_PipelineLayout;
		std::unique_ptr<giPipeline> m_Pipeline;

		VkCommandPool m_CommandPool;

		VkImage m_DepthImage;
		VkDeviceMemory m_DepthImageMemory;
		VkImageView m_DepthImageView;

		std::vector<VkBuffer> m_UniformBuffers;
		std::vector<VkDeviceMemory> m_UniformBuffersMemories;
		std::vector<void *> m_UniformBuffersMapped;

		VkDescriptorPool m_DescriptorPool;
		std::vector<VkDescriptorSet> m_DescriptorSets;

		std::vector<VkCommandBuffer> m_CommandBuffers;

		std::vector<VkSemaphore> m_ImageAvaliableSemaphores;
		std::vector<VkSemaphore> m_RenderFinishedSemaphores;
		std::vector<VkFence> m_InFlightFences;

#if USE_DEBUG_DRAWING
		uint32_t m_DDLastFramePointsDrawCallHash;
		uint32_t m_DDCurrentFramePointsDrawCallHash;

		uint32_t m_DDLastFrameLinesDrawCallHash;
		uint32_t m_DDCurrentFrameLinesDrawCallHash;

		uint32_t m_DDPointsCount = 0;
		std::vector<Vertex> m_DDPoints{};

		uint32_t m_DDLinesCount = 0;
		std::vector<Vertex> m_DDLines{};

		VkBuffer m_DDPointsBuffer;
		VkDeviceMemory m_DDPointsMemory;

		VkBuffer m_DDLinesBuffer;
		VkDeviceMemory m_DDLinesMemory;
#endif

		//----------------------------------------------

		// First rendered entity in the chain of all rendered entity (linked list). Use entity->pNextRenderedEntity for next element in the list.
		// If this is null, there are no rendered entity. If next is null, it is the last rendered entity.
		RenderedEntity *m_pFirstRenderedEntity{};

		Light *m_pFirstLight{};

		const Camera *m_pCamera = nullptr;


		std::string m_VertShaderFilePath{"assets/shaders/simple_shader.vert.spv"};
		std::string m_FragShaderFilePath{"assets/shaders/simple_shader.frag.spv"};

		float m_RenderTime = 0.0f;
	};
}
#endif