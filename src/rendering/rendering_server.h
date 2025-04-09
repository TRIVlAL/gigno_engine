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

	const size_t MAX_FRAMES_IN_FLIGHT = 2;
	const int MAX_LIGHT_DATA_COUNT = 15;
	const size_t SHADOW_MAP_CASCADE_COUNT = 3;

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

		size_t GetShadowMapCascadeCount() { return m_ShadowMapPass.CascadeCount; }

		//Debug Drawing ( need to active USE_DEBUG_DRAWING in features_usage.h )
		void DrawPoint(glm::vec3 pos, glm::vec3 color);
		void DrawLine(glm::vec3 startPos, glm::vec3 endPos, glm::vec3 color);
		void DrawLineGradient(glm::vec3 startPos, glm::vec3 endPos, glm::vec3 startColor, glm::vec3 endColor);

		#if USE_DEBUG_DRAWING
		bool ShowDD = true;
		bool ShowDDPoints = true;
		bool ShowDDLines = true;
		#endif

		typedef uint32_t RenderingParameters_t;
		enum RenderingParametersFlags_t {
			RP_FULLBRIGHT_BITS_POSITION = 0,
			RP_FULLBRIGHT_BITS_COUNT = 2,

			RP_SHADOW_MAP_ENABLE_BIT_POSITION = RP_FULLBRIGHT_BITS_POSITION + RP_FULLBRIGHT_BITS_COUNT,
			RP_SHADOW_MAP_ENABLE_BIT_COUNT = 1,

			RP_SHADOW_MAP_SAMPLE_COUNT_BIT_POSITION = RP_SHADOW_MAP_ENABLE_BIT_POSITION + RP_SHADOW_MAP_ENABLE_BIT_COUNT,
			RP_SHADOW_MAP_SAMPLE_COUNT_BIT_COUNT = 3,

			RP_SHADOW_MAP_APPLICATION_RANGE_DEBUG_BIT_POSITION = RP_SHADOW_MAP_SAMPLE_COUNT_BIT_POSITION + RP_SHADOW_MAP_SAMPLE_COUNT_BIT_COUNT,
			RP_SHADOW_MAP_APPLICATION_RANGE_DEBUG_BIT_COUNT = 1
		};
	private:


		/*
		Push COnstant --------------------------------------
		Per-Entity data pushed to the shader.
		Implementation :
			* Must follow the Vulkan alignment rules : (From vulkan-tutorial) https://vulkan-tutorial.com/Uniform_buffers/Descriptor_pool_and_sets#page_Alignment-requirements
		---------------------------------------------------*/
		class PushConstants {
		public:
			struct MainRender_t {
				glm::mat4 modelMatrix;
				glm::mat4 normalsMatrix;
			};

			struct ShadowMapRender_t {
				glm::mat4 modelMatrix;
			};
		};

		/*
		Uniform Buffer --------------------------------------
			Constant-during-the-frame data pushed to the shader.
			Implementation :
				* Must follow the Vulkan alignment rules : (From vulkan-tutorial) https://vulkan-tutorial.com/Uniform_buffers/Descriptor_pool_and_sets#page_Alignment-requirements
		------------------------------------------------------
		*/
		class UniformBuffers {
		public:
			struct MainRender_t {
				glm::mat4 View{1.0f};
				glm::mat4 Projection{1.0f};
				glm::vec4 LightDatas[MAX_LIGHT_DATA_COUNT];

				//Shadow Map transformations
				glm::mat4 LightView[SHADOW_MAP_CASCADE_COUNT];
				glm::mat4 LightProjection[SHADOW_MAP_CASCADE_COUNT];

				alignas(4) RenderingParameters_t Parameters;
			};

			struct ShadowMapRender_t {
				glm::mat4 LightView{1.0f};
				glm::mat4 LightProjection{1.0f};
			};
		};

		struct SceneRenderingData_t {
			RenderedEntity * RenderedEntities; // First entity in the chain.
			Light *LightEntities; // First light in the chain.
			const Camera *pCamera;
		};


		/*
		--------------------------------------------
		INITIALIZATION
		--------------------------------------------
		*/
		void CreateSyncObjects();
		
		void CreateVkSwapChain(bool isFirstCreation);
		void CreateImageViews();
		void CreateMainRenderPass();
		void CreateShadowMapRenderPass();
		void CreateDescriptorSetLayouts();
		void CreatePipelines(bool isRecreate = false);
		void CreateCommandPool();
		void CreateShadowMapSamplers();
		void CreateDepthResources(bool isRecreate = false);
		void CreateVariancedShadowmapImages();
		void CreateFrameBuffers();
		void CreateUniformBuffers();
		void CreateDescriptorPool();
		void CreateDescriptorSets();
		void CreateCommandBuffers();

		void Recreate();

		void RecordCommandBuffer(SceneRenderingData_t &sceneData, uint32_t imageIndex);
		
		//RECORD COMMAND BUFFER --------------------------------

		// Shadowmap Pass
		void ShadowMap_UniformBufferCommands(SceneRenderingData_t &sceneData, size_t cascadeIndex);
		void ShadowMap_RenderEntitiesCommands(SceneRenderingData_t &sceneData);
		void ShadowMap_RenderVariancedImage(size_t cascadeIndex);

		// Main Render
		void Main_UniformBufferCommands(const Camera *camera, Light *lights);
		void Main_RenderEntitiesCommands(RenderedEntity *entities);
	#if USE_DEBUG_DRAWING
		void Main_RenderDebugDrawingsCommands(const Camera *camera);
	#endif
		//-----------------------------------------------------

	#if USE_DEBUG_DRAWING
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
		std::vector<char> ReadFile(std::string filePath);
		VkShaderModule CreateShaderModule(const std::vector<char> &code);
		uint32_t GetMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties);

		uint32_t m_CurrentFrame = 0;

		//------------------------------------------------------

		Window m_Window;
		Device m_Device;

		struct UniformBuffer {
			VkBuffer Buffer;
			VkDeviceMemory Memory;
			void *Mapped;
		};

		struct {
			VkSwapchainKHR SwapChain;
			std::vector<VkImage> Images;
			VkFormat Format;
			VkExtent2D Extent;
			std::vector<VkImageView> ImageViews;
			std::vector<VkFramebuffer> Framebuffers;

			std::vector<UniformBuffer> UniformBuffers;
		} m_SwapChain;

		VkCommandPool m_CommandPool;

		struct FramebufferAttachment_t {
			VkImage Image;
			VkDeviceMemory Memory;
			VkImageView View;
		};

		struct {
			FramebufferAttachment_t DepthAttachment;
			VkRenderPass RenderPass;
			VkPipelineLayout PipelineLayout;
			VkDescriptorSetLayout DescriptorSetLayout;
			VkPipeline Pipeline;
		} m_MainPass;

		struct {
			uint32_t CascadeCount = SHADOW_MAP_CASCADE_COUNT;
			uint32_t Width = 2000, Height = 2000;

			std::vector<VkFramebuffer> DepthFramebuffers;
			std::vector<VkFramebuffer> VariancedFramebuffers;

			std::vector<FramebufferAttachment_t> DepthAttachments;
			std::vector<FramebufferAttachment_t> VariancedAttachments;

			VkRenderPass DepthRenderPass;
			VkRenderPass VariancedRenderPass;

			std::vector<VkSampler> DepthSamplers;
			std::vector<VkSampler> VariancedSamplers;

			std::vector<VkDescriptorImageInfo> DepthImageInfos;
			std::vector<VkDescriptorImageInfo> VariancedImageInfos;

			VkPipelineLayout DepthPipelineLayout;
			VkPipelineLayout VariancedPipelineLayout;
			
			VkDescriptorSetLayout DepthDescriptorSetLayout;
			VkDescriptorSetLayout VariancedDescriptorSetLayout;

			VkPipeline DepthPipeline;
			VkPipeline VariancedPipeline;

			std::vector<UniformBuffer> UniformBuffers;
		} m_ShadowMapPass;


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

		// First light in the chain of all Lights. The Chain ALWAYS STARTS with the directional lights. The rest follows in no particular order.
		Light *m_pFirstLight{};

		const Camera *m_pCamera = nullptr;


		std::string m_VertShaderFilePath{"assets/shaders/simple_shader.vert.spv"};
		std::string m_FragShaderFilePath{"assets/shaders/simple_shader.frag.spv"};

		float m_RenderTime = 0.0f;
	};
}
#endif