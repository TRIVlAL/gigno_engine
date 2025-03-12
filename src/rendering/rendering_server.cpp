#include "rendering_server.h"
#include "../error_macros.h"
#include "../entities/lights/light.h"

#include "application.h"

#include "../features_usage.h"
#if USE_IMGUI
	#include "gui.h"
#endif

#include "rendering_utils.h"
#include "iostream"
#include "../debug/console/convar.h"

namespace gigno {

	CONVAR(int, r_fullbright, 0, "1 : no lighting applied. 2 : No color but do lighting");

	void RenderingServer::Init(int winw, int winh, const char *winTitle) {
		m_Window.Init(winw, winh, winTitle);
		m_Device.Init(&m_Window);

		//m_SwapChain.Init(m_Device, &m_Window, m_VertShaderFilePath, m_FragShaderFilePath);
		CreateVkSwapChain(true);
		CreateImageViews();
		CreateRenderPass();
		CreateDescriptorSetLayout();
		CreatePipeline();
		CreateCommandPool();
		CreateDepthResources();
		CreateFrameBuffers();
		CreateUniformBuffers();
		CreateDescriptorPool();
		CreateDescriptorSets();
		CreateCommandBuffers();
		CreateSyncObjects();

		#if USE_IMGUI
		GLFWwindow *glfwWindow = m_Window.GetGLFWwindow();
		if(glfwWindow){
			InitImGui(glfwWindow, m_Device, m_RenderPass);
		}
		#endif
	}

	void RenderingServer::Recreate() {
		int width = 0, height = 0;
		m_Window.GetFrameBufferSize(&width, &height);
		while (width == 0 || height == 0) {
			m_Window.GetFrameBufferSize(&width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(m_Device.GetDevice());

		for (auto imageView : m_SwapChain.ImageViews) {
			vkDestroyImageView(m_Device.GetDevice(), imageView, nullptr);
		}
		for (auto frameBuffer : m_SwapChain.Framebuffers) {
			vkDestroyFramebuffer(m_Device.GetDevice(), frameBuffer, nullptr);
		}

		vkDestroyImageView(m_Device.GetDevice(), m_DepthImageView, nullptr);
		vkDestroyImage(m_Device.GetDevice(), m_DepthImage, nullptr);
		vkFreeMemory(m_Device.GetDevice(), m_DepthImageMemory, nullptr);
		vkDestroyPipelineLayout(m_Device.GetDevice(), m_PipelineLayout, nullptr);
		
		CreateVkSwapChain(false);
		CreateImageViews();
		CreateDepthResources();
		CreateFrameBuffers();
		CreatePipeline();
	}

    RenderingServer::~RenderingServer() {
		
		vkDeviceWaitIdle(m_Device.GetDevice());

#if USE_IMGUI
		ShutdownImGui();
#endif

		for (auto imageView : m_SwapChain.ImageViews) {
			vkDestroyImageView(m_Device.GetDevice(), imageView, nullptr);
		}
		for (auto frameBuffer : m_SwapChain.Framebuffers) {
			vkDestroyFramebuffer(m_Device.GetDevice(), frameBuffer, nullptr);
		}
		vkDestroyImageView(m_Device.GetDevice(), m_DepthImageView, nullptr);
		vkDestroyImage(m_Device.GetDevice(), m_DepthImage, nullptr);
		vkFreeMemory(m_Device.GetDevice(), m_DepthImageMemory, nullptr);
		vkDestroySwapchainKHR(m_Device.GetDevice(), m_SwapChain.SwapChain, nullptr);

		vkDestroyPipeline(m_Device.GetDevice(), m_Pipeline, nullptr);
		vkDestroyPipelineLayout(m_Device.GetDevice(), m_PipelineLayout, nullptr);
		vkDestroyRenderPass(m_Device.GetDevice(), m_RenderPass, nullptr);
		for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroyBuffer(m_Device.GetDevice(), m_UniformBuffers[i], nullptr);
			vkFreeMemory(m_Device.GetDevice(), m_UniformBuffersMemories[i], nullptr);
		}

		vkDestroyDescriptorPool(m_Device.GetDevice(), m_DescriptorPool, nullptr); 
		vkDestroyDescriptorSetLayout(m_Device.GetDevice(), m_DescriptorSetLayout, nullptr);
		
		for(rsize_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(m_Device.GetDevice(), m_ImageAvaliableSemaphores[i], nullptr);
			vkDestroySemaphore(m_Device.GetDevice(), m_RenderFinishedSemaphores[i], nullptr);
			vkDestroyFence(m_Device.GetDevice(), m_InFlightFences[i], nullptr);
		}

		vkDestroyCommandPool(m_Device.GetDevice(), m_CommandPool, nullptr);
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
		Console::LogError("Tried to unsubsribe rendered entity '%s' but it was not subscribed.", entity->Name == "" ? "No name" : entity->Name);
	}

	void RenderingServer::SubscribeLightEntity(Light *light)
	{
		light->pNextLight = m_pFirstLight;
		m_pFirstLight = light;
	}

	void RenderingServer::UnsubscribeLightEntity(Light *light)
	{
		Light *curr = m_pFirstLight;
		if(light == curr) {
			m_pFirstLight = curr->pNextLight;
			return;
		}
		while(curr) {
			if(curr->pNextLight == light) {
				curr->pNextLight = light->pNextLight;
				return;
			}
			curr = curr->pNextLight;
		}
	}

	void RenderingServer::CreateModel(std::shared_ptr<giModel> &model, const ModelData_t &modelData) {
		model = std::make_shared<giModel>(giModel{ m_Device, modelData, m_CommandPool });
	}

    void RenderingServer::ClenUpModel(std::shared_ptr<giModel> &model) {
		model->CleanUp(m_Device.GetDevice());
    }


	/*
	*
	*
	*
	*
	*
	---------------------------------------------------------------
	RENDER
	---------------------------------------------------------------'
	*
	*
	*
	*
	*
	*/


	void RenderingServer::Render() {
		DrawFrame();
	}

	void RenderingServer::DrawFrame() {
		if(!m_pCamera) {
			Console::LogWarning("No Active Camera in the scene !");
		}

		vkWaitForFences(m_Device.GetDevice(), 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

		uint32_t image_index = 0;
		VkResult result = vkAcquireNextImageKHR(m_Device.GetDevice(), m_SwapChain.SwapChain, UINT64_MAX, m_ImageAvaliableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &image_index);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_Window.HasResized()) {
			Recreate();
			return;
		}
		else if (result != VK_SUCCESS) {
			Console::LogError("Failed to Acquire Swap Chain Image ! Vulkan Error Code : %d", (int) result);
		}

		vkResetFences(m_Device.GetDevice(), 1, &m_InFlightFences[m_CurrentFrame]);

		vkResetCommandBuffer(m_CommandBuffers[m_CurrentFrame], 0);

		#if USE_DEBUG_DRAWING
		UpdateDebugDrawings();
		#endif

		SceneRenderingData_t scene_data{m_pFirstRenderedEntity, m_pFirstLight, m_pCamera};
		RecordCommandBuffer(scene_data, image_index);

		VkSemaphore wait_semaphores[] = { m_ImageAvaliableSemaphores[m_CurrentFrame]};
		VkSemaphore signal_semaphores[] = { m_RenderFinishedSemaphores[m_CurrentFrame]};
		VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = wait_semaphores;
		submitInfo.pWaitDstStageMask = wait_stages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_CommandBuffers[m_CurrentFrame];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signal_semaphores;

		vkResetFences(m_Device.GetDevice(), 1, &m_InFlightFences[m_CurrentFrame]);

		result = vkQueueSubmit(m_Device.GetGraphicsQueue(), 1, &submitInfo, m_InFlightFences[m_CurrentFrame]);
		if (result != VK_SUCCESS) {
			Console::LogError("Failed to Submit to Graphics Queue ! Vulkan Error Code : %d", (int)result);
		}

		VkSwapchainKHR swapchains[] = { m_SwapChain.SwapChain };

		VkPresentInfoKHR present_info{};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = signal_semaphores;
		present_info.swapchainCount = 1;
		present_info.pSwapchains = swapchains;
		present_info.pImageIndices = &image_index;
		present_info.pResults = nullptr;

		result = vkQueuePresentKHR(m_Device.GetPresentQueue(), &present_info);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			Recreate();
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			Console::LogError("Failed to Present Swap Chain Image ! Vulkan Error Code : %d", (int)result);
		} 

		m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

	#if USE_IMGUI
		NewFrameImGui();
	#endif
	}

	void RenderingServer::RecordCommandBuffer(SceneRenderingData_t &sceneData, uint32_t imageIndex) {
		VkCommandBufferBeginInfo buffer_begin_info{};
		buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		buffer_begin_info.flags = 0;
		buffer_begin_info.pInheritanceInfo = nullptr;

		VULKAN_CHECK(vkBeginCommandBuffer(m_CommandBuffers[m_CurrentFrame], &buffer_begin_info),
						"Failed to begin Command Buffer ! ");

		std::array<VkClearValue, 2> clear_vals = {};
		clear_vals[0].color = {{.15f, 0.15f, 0.15f, 1.0f}};
		clear_vals[1].depthStencil = {1.0f, 0};

		VkRenderPassBeginInfo pass_begin_info{};
		pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		pass_begin_info.renderPass = m_RenderPass;
		pass_begin_info.framebuffer = m_SwapChain.Framebuffers[imageIndex];
		pass_begin_info.renderArea.offset = {0, 0};
		pass_begin_info.renderArea.extent = m_SwapChain.Extent;
		pass_begin_info.clearValueCount = static_cast<uint32_t>(clear_vals.size());
		pass_begin_info.pClearValues = clear_vals.data();

		vkCmdBeginRenderPass(m_CommandBuffers[m_CurrentFrame], &pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_SwapChain.Extent.width);
		viewport.height = static_cast<float>(m_SwapChain.Extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(m_CommandBuffers[m_CurrentFrame], 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.extent = m_SwapChain.Extent;
		scissor.offset = { 0, 0 };
		vkCmdSetScissor(m_CommandBuffers[m_CurrentFrame], 0, 1, &scissor);

		if (sceneData.pCamera) {
			UpdateUniformBuffer(sceneData.pCamera, sceneData.LightEntities);
			RenderEntities(sceneData.RenderedEntities);
			#if USE_DEBUG_DRAWING
			RenderDebugDrawings(sceneData.pCamera);
			#endif
		}


		#if USE_IMGUI
		RenderImGui(m_CommandBuffers[m_CurrentFrame]);
		#endif

		vkCmdEndRenderPass(m_CommandBuffers[m_CurrentFrame]);

		VULKAN_CHECK(vkEndCommandBuffer(m_CommandBuffers[m_CurrentFrame]),
					"Failed to end Command Buffer ! ");
	}

    void RenderingServer::UpdateUniformBuffer(const Camera *camera, Light *lights) {
		const glm::mat4 proj = camera->GetProjection();
		const glm::mat4 view = camera->GetViewMatrix();

		UniformBufferData_t ub{};
		ub.projection = proj;
		ub.view = view;

		uint32_t i = 0;
		Light *light = lights;
		while(light) {
			const uint32_t advance = light->DataSlotsCount();
			if (i + advance > MAX_LIGHT_DATA_COUNT)
			{
				Console::LogError("Rendering : Max Light Data exceeded ! Some lights will be ignored !");
				break;
			}
			light->FillDataSlots(&ub.lightDatas[i]);
			i += advance;
			light = light->pNextLight;
		}

		std::memcpy(m_UniformBuffersMapped[m_CurrentFrame], &ub, sizeof(ub));

		vkCmdBindDescriptorSets(m_CommandBuffers[m_CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, 
								&m_DescriptorSets[m_CurrentFrame], 0, nullptr);
    }

    void RenderingServer::RenderEntities(RenderedEntity *entities) {
		vkCmdSetPrimitiveTopology(m_CommandBuffers[m_CurrentFrame], VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

		vkCmdBindPipeline(m_CommandBuffers[m_CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);

		RenderedEntity *curr = entities;
		while(curr) {
			const std::shared_ptr<giModel> model = curr->GetModel();
			if(model != nullptr) {
				PushConstantData_t push{};
				push.modelMatrix = curr->TransformationMatrix();
				push.normalsMatrix = glm::mat4{curr->NormalMatrix()};
				push.fullbright = (int)convar_r_fullbright;
				vkCmdPushConstants(m_CommandBuffers[m_CurrentFrame], m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantData_t), &push);

				model->Bind(m_CommandBuffers[m_CurrentFrame]);
				model->Draw(m_CommandBuffers[m_CurrentFrame]);
			}

			curr = curr->pNextRenderedEntity;
		}
    }

    void RenderingServer::RenderDebugDrawings(const Camera *camera)
    {
        PushConstantData_t push{};
		push.modelMatrix = {1.0f};
		push.normalsMatrix = {1.0f};
		push.fullbright = true;
		vkCmdPushConstants(m_CommandBuffers[m_CurrentFrame], m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantData_t), &push);

		VkDeviceSize offset = 0;
		
		if(m_DDPointsCount > 0) {
			vkCmdSetPrimitiveTopology(m_CommandBuffers[m_CurrentFrame], VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
			vkCmdBindVertexBuffers(m_CommandBuffers[m_CurrentFrame], 0, 1, &m_DDPointsBuffer, &offset);
			vkCmdDraw(m_CommandBuffers[m_CurrentFrame], m_DDPointsCount, 1, 0, 0);
		}

		if(m_DDLinesCount > 0) {
			vkCmdSetPrimitiveTopology(m_CommandBuffers[m_CurrentFrame], VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
			vkCmdBindVertexBuffers(m_CommandBuffers[m_CurrentFrame], 0, 1, &m_DDLinesBuffer, &offset);
			vkCmdDraw(m_CommandBuffers[m_CurrentFrame], m_DDLinesCount, 1, 0, 0);
		}
    }


	/*
	*
	*
	*
	*
	*
	*
    --------------------------------------------------------------------
	Creation
	--------------------------------------------------------------------
	*
	*
	*
	*
	*
	*
	*/

	void RenderingServer::CreateVkSwapChain(bool isFirstCreation) 
	{
		SwapChainSupportDetails support_details = m_Device.GetPhysicalSwapChainSupport();
		QueueFamilyIndices queue_family_indices = m_Device.GetPhysicalDeviceQueueFamilyIndices();

		VkSurfaceFormatKHR surface_fmt = ChooseSwapSurfaceFormat(support_details.formats);
		VkPresentModeKHR present_mode = ChooseSwapPresentMode(support_details.presentModes);
		VkExtent2D extent = ChooseSwapExtent(&m_Window, support_details.surfaceCapabilities);

		m_SwapChain.Extent = extent;
		m_SwapChain.Format = surface_fmt.format;

		uint32_t imageCount = support_details.surfaceCapabilities.minImageCount + 1;
		if (support_details.surfaceCapabilities.maxImageCount > 0 && imageCount > support_details.surfaceCapabilities.maxImageCount) {
			imageCount = support_details.surfaceCapabilities.maxImageCount;
		}

		uint32_t queue_fam_indices[] = { queue_family_indices.graphicFamily.value(), queue_family_indices.presentFamily.value() };

		VkSwapchainKHR existing_swapchain = isFirstCreation ? VK_NULL_HANDLE : m_SwapChain.SwapChain;

		VkSwapchainCreateInfoKHR createinfo{};
		createinfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createinfo.pNext = nullptr;
		createinfo.surface = m_Device.GetSurface();
		createinfo.minImageCount = imageCount;
		createinfo.imageFormat = surface_fmt.format;
		createinfo.imageColorSpace = surface_fmt.colorSpace;
		createinfo.imageExtent = extent;
		createinfo.imageArrayLayers = 1;
		createinfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		if (queue_family_indices.graphicFamily != queue_family_indices.presentFamily) {
			createinfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createinfo.queueFamilyIndexCount = 2;
			createinfo.pQueueFamilyIndices = queue_fam_indices;
		}
		else {
			createinfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createinfo.queueFamilyIndexCount = 0;
			createinfo.pQueueFamilyIndices = nullptr;
		}
		createinfo.preTransform = support_details.surfaceCapabilities.currentTransform;
		createinfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createinfo.presentMode = present_mode;
		createinfo.clipped = VK_TRUE;
		createinfo.oldSwapchain = existing_swapchain;

		VkResult result = vkCreateSwapchainKHR(m_Device.GetDevice(), &createinfo, nullptr, &m_SwapChain.SwapChain);
		if (result != VK_SUCCESS) {
			Console::LogError("Failed to create Vulkan Swapchain ! Vulkan Errror Code : %d", (int)result);
		}

		if(existing_swapchain != VK_NULL_HANDLE) {
			vkDestroySwapchainKHR(m_Device.GetDevice(), existing_swapchain, nullptr);
		}

		uint32_t images_count = 0;
		vkGetSwapchainImagesKHR(m_Device.GetDevice(), m_SwapChain.SwapChain, &images_count, nullptr);
		m_SwapChain.Images.resize(images_count);

		vkGetSwapchainImagesKHR(m_Device.GetDevice(), m_SwapChain.SwapChain, &images_count, m_SwapChain.Images.data());
	}

    void RenderingServer::CreateImageViews() {
		m_SwapChain.ImageViews.resize(m_SwapChain.Images.size());
		for (size_t i = 0; i < m_SwapChain.Images.size(); i++) {
			m_SwapChain.ImageViews[i] = CreateImageView(m_Device.GetDevice(), m_SwapChain.Images[i], m_SwapChain.Format, VK_IMAGE_ASPECT_COLOR_BIT);
		}
	}

    void RenderingServer::CreateRenderPass() {
		VkAttachmentDescription color_attachment{};
		color_attachment.format = m_SwapChain.Format;
		color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference color_attachment_ref{};
		color_attachment_ref.attachment = 0;
		color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depth_attachment{};
		depth_attachment.format = FindDepthFormat(m_Device.GetPhysicalDevice());
		depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depth_attachment_ref{};
		depth_attachment_ref.attachment = 1;
		depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_attachment_ref;
		subpass.pDepthStencilAttachment = &depth_attachment_ref;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 2> attachments{color_attachment, depth_attachment};
		VkRenderPassCreateInfo createinfo{};
		createinfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		createinfo.pNext = nullptr;
		createinfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		createinfo.pAttachments = attachments.data();
		createinfo.subpassCount = 1;
		createinfo.pSubpasses = &subpass;
		createinfo.dependencyCount = 1;
		createinfo.pDependencies = &dependency;
		
		VULKAN_CHECK(vkCreateRenderPass(m_Device.GetDevice(), &createinfo, nullptr, &m_RenderPass), 
						"Failed to create Render Pass ! ");
    }

    void RenderingServer::CreateDescriptorSetLayout() {
		VkDescriptorSetLayoutBinding ubo_layout_binding{};
		ubo_layout_binding.binding = 0;
		ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		ubo_layout_binding.descriptorCount = 1;
		ubo_layout_binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

		VkDescriptorSetLayoutCreateInfo createinfo{};
		createinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createinfo.pNext = nullptr;
		createinfo.flags = 0;
		createinfo.bindingCount = 1;
		createinfo.pBindings = &ubo_layout_binding;

		VULKAN_CHECK(vkCreateDescriptorSetLayout(m_Device.GetDevice(), &createinfo, nullptr, &m_DescriptorSetLayout),
						"Failed to create Descriptor Set Layout ! ");
    }

    void RenderingServer::CreatePipeline() {
		VkShaderModule vert_shader_module = CreateShaderModule(ReadFile(m_VertShaderFilePath));
		VkShaderModule frag_shader_module = CreateShaderModule(ReadFile(m_FragShaderFilePath));

		VkPipelineShaderStageCreateInfo vert_shader_stage_info{};
		vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vert_shader_stage_info.module = vert_shader_module;
		vert_shader_stage_info.pName = "main";

		VkPipelineShaderStageCreateInfo frag_shader_stage_info{};
		frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		frag_shader_stage_info.module = frag_shader_module;
		frag_shader_stage_info.pName = "main";

		VkPipelineShaderStageCreateInfo shader_stages[] = {vert_shader_stage_info, frag_shader_stage_info};

		VkPushConstantRange push_constant_range{};
		push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		push_constant_range.offset = 0;
		push_constant_range.size = sizeof(PushConstantData_t);

		VkPipelineLayoutCreateInfo pipeline_layout_info{};
		pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_info.pNext = nullptr;
		pipeline_layout_info.setLayoutCount = 1;
		pipeline_layout_info.pSetLayouts = &m_DescriptorSetLayout;
		pipeline_layout_info.pushConstantRangeCount = 1;
		pipeline_layout_info.pPushConstantRanges = &push_constant_range;

		VULKAN_CHECK(vkCreatePipelineLayout(m_Device.GetDevice(), &pipeline_layout_info, nullptr, &m_PipelineLayout),
					 "Failed to create Vulkan Pipeline Layout ! ");

		VkVertexInputBindingDescription bindingDescription = Vertex::GetBindingDescription();
		std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions = Vertex::GetAttributeDescriptions();

		VkPipelineVertexInputStateCreateInfo vertex_input_info{};
		vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_input_info.pNext = nullptr;
		vertex_input_info.vertexBindingDescriptionCount = 1;
		vertex_input_info.pVertexBindingDescriptions = &bindingDescription;
		vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertex_input_info.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo input_assembly_info{};
		input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		input_assembly_info.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewport_state_info{};
		viewport_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_state_info.pNext = nullptr;
		viewport_state_info.viewportCount = 1;
		viewport_state_info.pViewports = nullptr;
		viewport_state_info.scissorCount = 1;
		viewport_state_info.pScissors = nullptr;

		VkPipelineRasterizationStateCreateInfo rasterization_state_info{};
		rasterization_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterization_state_info.depthClampEnable = VK_FALSE;
		rasterization_state_info.rasterizerDiscardEnable = VK_FALSE;
		rasterization_state_info.polygonMode = VK_POLYGON_MODE_FILL;
		rasterization_state_info.lineWidth = 4.0f;
		rasterization_state_info.cullMode = VK_CULL_MODE_NONE;
		rasterization_state_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterization_state_info.depthBiasEnable = VK_FALSE;
		rasterization_state_info.depthBiasConstantFactor = 0.0f;
		rasterization_state_info.depthBiasClamp = 0.0f;
		rasterization_state_info.depthBiasSlopeFactor = 0.0f;

		VkPipelineMultisampleStateCreateInfo multisample_state_info{};
		multisample_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisample_state_info.sampleShadingEnable = VK_FALSE;
		multisample_state_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisample_state_info.minSampleShading = 1.0f;
		multisample_state_info.pSampleMask = nullptr;
		multisample_state_info.alphaToCoverageEnable = VK_FALSE;
		multisample_state_info.alphaToOneEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState color_blend_state_attachment{};
		color_blend_state_attachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;
		color_blend_state_attachment.blendEnable = VK_FALSE;
		color_blend_state_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_state_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		color_blend_state_attachment.colorBlendOp = VK_BLEND_OP_ADD;
		color_blend_state_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_state_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		color_blend_state_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo color_blend_state_info{};
		color_blend_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blend_state_info.logicOpEnable = VK_FALSE;
		color_blend_state_info.logicOp = VK_LOGIC_OP_COPY;
		color_blend_state_info.attachmentCount = 1;
		color_blend_state_info.pAttachments = &color_blend_state_attachment;
		color_blend_state_info.blendConstants[0] = 0.0f;
		color_blend_state_info.blendConstants[1] = 0.0f;
		color_blend_state_info.blendConstants[2] = 0.0f;
		color_blend_state_info.blendConstants[3] = 0.0f;

		VkPipelineDepthStencilStateCreateInfo depth_stencil_state_info{};
		depth_stencil_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depth_stencil_state_info.depthTestEnable = VK_TRUE;
		depth_stencil_state_info.depthWriteEnable = VK_TRUE;
		depth_stencil_state_info.depthCompareOp = VK_COMPARE_OP_LESS;
		depth_stencil_state_info.depthBoundsTestEnable = VK_FALSE;
		depth_stencil_state_info.minDepthBounds = 0.0f;
		depth_stencil_state_info.maxDepthBounds = 1.0f;
		depth_stencil_state_info.stencilTestEnable = VK_FALSE;
		depth_stencil_state_info.front = {};
		depth_stencil_state_info.back = {};


		std::array<VkDynamicState, 3> dynamic_states_enables{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY};

		VkPipelineDynamicStateCreateInfo dynamic_state_info{};
		dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_state_info.flags = 0;
		dynamic_state_info.pNext = nullptr;
		dynamic_state_info.dynamicStateCount = static_cast<uint32_t>(dynamic_states_enables.size());
		dynamic_state_info.pDynamicStates = dynamic_states_enables.data();

		VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
		graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		graphicsPipelineCreateInfo.pNext = nullptr;
		graphicsPipelineCreateInfo.stageCount = 2;
		graphicsPipelineCreateInfo.pStages = shader_stages;
		graphicsPipelineCreateInfo.pVertexInputState = &vertex_input_info;
		graphicsPipelineCreateInfo.pInputAssemblyState = &input_assembly_info;
		graphicsPipelineCreateInfo.pViewportState = &viewport_state_info;
		graphicsPipelineCreateInfo.pRasterizationState = &rasterization_state_info;
		graphicsPipelineCreateInfo.pMultisampleState = &multisample_state_info;
		graphicsPipelineCreateInfo.pDepthStencilState = &depth_stencil_state_info;
		graphicsPipelineCreateInfo.pColorBlendState = &color_blend_state_info;
		graphicsPipelineCreateInfo.pDynamicState = &dynamic_state_info;

		graphicsPipelineCreateInfo.layout = m_PipelineLayout;
		graphicsPipelineCreateInfo.renderPass = m_RenderPass;
		graphicsPipelineCreateInfo.subpass = 0;

		graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
		graphicsPipelineCreateInfo.basePipelineIndex = -1;

		VULKAN_CHECK(vkCreateGraphicsPipelines(m_Device.GetDevice(), nullptr, 1, &graphicsPipelineCreateInfo, nullptr, &m_Pipeline),
					"Failed to create Graphics Pipeline ! ");

		vkDestroyShaderModule(m_Device.GetDevice(), vert_shader_module, nullptr);
		vkDestroyShaderModule(m_Device.GetDevice(), frag_shader_module, nullptr);
	}

    void RenderingServer::CreateCommandPool() {
		VkCommandPoolCreateInfo createinfo{};
		createinfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createinfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		createinfo.queueFamilyIndex = m_Device.GetPhysicalDeviceQueueFamilyIndices().graphicFamily.value();

		VULKAN_CHECK(vkCreateCommandPool(m_Device.GetDevice(), &createinfo, nullptr, &m_CommandPool),
					"Failed to create Vulkan Command Pool ! ");
    }

    void RenderingServer::CreateDepthResources() {
		VkFormat format = FindDepthFormat(m_Device.GetPhysicalDevice());

		CreateImage(m_Device.GetDevice(), m_Device.GetPhysicalDevice(), m_SwapChain.Extent.width, m_SwapChain.Extent.height, format, 
		VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_DepthImage, m_DepthImageMemory);
		
		m_DepthImageView = CreateImageView(m_Device.GetDevice(), m_DepthImage, format, VK_IMAGE_ASPECT_DEPTH_BIT);
    }

    void RenderingServer::CreateFrameBuffers() {
		m_SwapChain.Framebuffers.resize(m_SwapChain.ImageViews.size());
		for (size_t i = 0; i < m_SwapChain.ImageViews.size(); i++) {
			std::array<VkImageView, 2> attachments = {m_SwapChain.ImageViews[i], m_DepthImageView};

			VkFramebufferCreateInfo createinfo{};
			createinfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			createinfo.renderPass = m_RenderPass;
			createinfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			createinfo.pAttachments = attachments.data();
			createinfo.width = m_SwapChain.Extent.width;
			createinfo.height = m_SwapChain.Extent.height;
			createinfo.layers = 1;

			VULKAN_CHECK(vkCreateFramebuffer(m_Device.GetDevice(), &createinfo, nullptr, &m_SwapChain.Framebuffers[i]),
							"Failed to create Vulkan Frame Buffer #%d ! ");
		}
    }

	void RenderingServer::CreateUniformBuffers() {
		VkDeviceSize size = sizeof(UniformBufferData_t);

		m_UniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		m_UniformBuffersMemories.resize(MAX_FRAMES_IN_FLIGHT);
		m_UniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

		for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			CreateBuffer(m_Device.GetDevice(), m_Device.GetPhysicalDevice(), size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_UniformBuffers[i], m_UniformBuffersMemories[i]);
			vkMapMemory(m_Device.GetDevice(), m_UniformBuffersMemories[i], 0, size, 0, &m_UniformBuffersMapped[i]);
		}
	}

    void RenderingServer::CreateDescriptorPool() {
		VkDescriptorPoolSize size{};
		size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		size.descriptorCount = MAX_FRAMES_IN_FLIGHT;

		VkDescriptorPoolCreateInfo createinfo{};
		createinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		createinfo.maxSets = MAX_FRAMES_IN_FLIGHT;
		createinfo.poolSizeCount = 1;
		createinfo.pPoolSizes = &size;

		VULKAN_CHECK(vkCreateDescriptorPool(m_Device.GetDevice(), &createinfo, nullptr, &m_DescriptorPool),
					"Failed to create Descriptor Pool ! ");
	}

    void RenderingServer::CreateDescriptorSets() {
		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_DescriptorSetLayout);
		VkDescriptorSetAllocateInfo allocinfo{};
		allocinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocinfo.descriptorPool = m_DescriptorPool;
		allocinfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
		allocinfo.pSetLayouts = layouts.data();

		m_DescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

		VULKAN_CHECK(vkAllocateDescriptorSets(m_Device.GetDevice(), &allocinfo, m_DescriptorSets.data()),
						"Failed to Allocate Descriptor Sets ! ");

		for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			VkDescriptorBufferInfo info{};
			info.buffer = m_UniformBuffers[i];
			info.offset = 0;
			info.range = sizeof(UniformBufferData_t);

			VkWriteDescriptorSet descWrite{};
			descWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descWrite.dstSet = m_DescriptorSets[i];
			descWrite.dstBinding = 0;
			descWrite.dstArrayElement = 0;
			descWrite.descriptorCount = 1;
			descWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descWrite.pBufferInfo = &info;

			vkUpdateDescriptorSets(m_Device.GetDevice(), 1, &descWrite, 0, nullptr);
		}
    }

    void RenderingServer::CreateCommandBuffers() {
		m_CommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocinfo{};
		allocinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocinfo.commandPool = m_CommandPool;
		allocinfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocinfo.commandBufferCount = (uint32_t)m_CommandBuffers.size();

		VULKAN_CHECK(vkAllocateCommandBuffers(m_Device.GetDevice(), &allocinfo, m_CommandBuffers.data()),
					"Failed to allocate Vulkan COmmand BUffers ! ");
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
				Console::LogError("Failed to create Vulkan Sync Object Semaphore / Fence.");
			}
		}
	}

	VkSurfaceFormatKHR RenderingServer::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &avaliableFormats) {
		for (const VkSurfaceFormatKHR &format : avaliableFormats) {
			if (format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && format.format == VK_FORMAT_B8G8R8A8_UNORM) {
				return format;
			}
		}
		return avaliableFormats[0];
	}

    VkPresentModeKHR RenderingServer::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &avaliablePresentModes) {
        for (const VkPresentModeKHR &mode : avaliablePresentModes) {
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return mode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D RenderingServer::ChooseSwapExtent(const Window *window, const VkSurfaceCapabilitiesKHR &capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		}
		else {
			int width;
			int height;
			window->GetFrameBufferSize(&width, &height);

			VkExtent2D actual_extent{
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actual_extent.width = std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actual_extent.height = std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actual_extent;
		}
    }

    VkFormat RenderingServer::FindSupportedFormat(VkPhysicalDevice physDevice, const std::vector<VkFormat> &candidates, VkImageTiling targetTiling, VkFormatFeatureFlags targetFeatures) {
        for (VkFormat format : candidates) {
			VkFormatProperties prop{};
			vkGetPhysicalDeviceFormatProperties(physDevice, format, &prop);
			if (targetTiling == VK_IMAGE_TILING_LINEAR && (prop.linearTilingFeatures & targetFeatures) == targetFeatures) {
				return format;
			}
			else if(targetTiling == VK_IMAGE_TILING_OPTIMAL && (prop.optimalTilingFeatures & targetFeatures) == targetFeatures) {
				return format;
			}
		}
		Console::LogError("Failed to find supported format !");
		return VkFormat{};
    }

    VkFormat RenderingServer::FindDepthFormat(VkPhysicalDevice physDevice) {
		return FindSupportedFormat(physDevice, {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
								   VK_IMAGE_TILING_OPTIMAL,
								   VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}

    uint32_t RenderingServer::FindMemoryTypeIndex(VkPhysicalDevice device, uint32_t typeFilter, VkMemoryPropertyFlags properties) const {
        VkPhysicalDeviceMemoryProperties mem_prop{};
		vkGetPhysicalDeviceMemoryProperties(device, &mem_prop);

		for (uint32_t i = 0; i < mem_prop.memoryTypeCount; i++) {
			if ((typeFilter & (i << i)) && (mem_prop.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		Console::LogError("Failed to find suitable memory type !");
		return UINT32_MAX;
    }

    void RenderingServer::CreateImage(VkDevice device, VkPhysicalDevice physDevice, uint32_t width, uint32_t height, VkFormat format, 
									VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags props, VkImage &image, VkDeviceMemory &imageMemory) {
										VkImageCreateInfo createinfo{};
		createinfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		createinfo.imageType = VK_IMAGE_TYPE_2D;
		createinfo.format = format;
		createinfo.extent.width = width;
		createinfo.extent.height = height;
		createinfo.extent.depth = 1;
		createinfo.mipLevels = 1;
		createinfo.arrayLayers = 1;
		createinfo.samples = VK_SAMPLE_COUNT_1_BIT;
		createinfo.tiling = tiling;
		createinfo.usage = usage;
		createinfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;


		VkResult result = vkCreateImage(device, &createinfo, nullptr, &image);
		if (result != VK_SUCCESS) {
			Console::LogError("Failed to create Vulkan Image ! Vulkan Errror Code : %d", (int)result);
		}

		VkMemoryRequirements mem_requirements;
		vkGetImageMemoryRequirements(device, image, &mem_requirements);

		VkMemoryAllocateInfo allocinfo{};
		allocinfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocinfo.allocationSize = mem_requirements.size;

		size_t mem_index = FindMemoryTypeIndex(physDevice, mem_requirements.memoryTypeBits, props);

		if(mem_index == UINT32_MAX) {
			result = VK_ERROR_UNKNOWN;
			allocinfo.memoryTypeIndex = 0;
		} else {
			allocinfo.memoryTypeIndex = result; 
			result = vkAllocateMemory(device, &allocinfo, nullptr, &imageMemory);
		}
		
		if (result != VK_SUCCESS) {
			Console::LogError("Failed to allocate Image Memory ! Vulkan Errror Code : %d", (int)result);
		}

		vkBindImageMemory(device, image, imageMemory, 0);
    }

    VkImageView RenderingServer::CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
        VkImageViewCreateInfo createinfo{};
		createinfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createinfo.pNext = nullptr;
		createinfo.image = image;
		createinfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createinfo.format = format;
		createinfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createinfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createinfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createinfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createinfo.subresourceRange.aspectMask = aspectFlags;
		createinfo.subresourceRange.baseMipLevel = 0;
		createinfo.subresourceRange.levelCount = 1;
		createinfo.subresourceRange.baseArrayLayer = 0;
		createinfo.subresourceRange.layerCount = 1;

		VkImageView ret{};
		VkResult result = vkCreateImageView(device, &createinfo, nullptr, &ret);
		if (result != VK_SUCCESS) {
			Console::LogError("Failed to create Vulkan Image View ! Vulkan Errror Code : %d", (int)result);
		}
		return ret;
    }

    std::vector<char> RenderingServer::ReadFile(std::string filePath)
    {
		std::ifstream infile{filePath, std::ios::ate | std::ios::binary};

		if (!infile.is_open())
		{
			Console::LogError("failed to open file: %s", filePath.c_str());
			return std::vector<char>{};
		}

		size_t filesize = static_cast<size_t>(infile.tellg());
		std::vector<char> buffer(filesize);
		infile.seekg(0);
		infile.read(buffer.data(), filesize);

		infile.close();
		return buffer;
	}

    VkShaderModule RenderingServer::CreateShaderModule(const std::vector<char> &code) {
        VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

		VkShaderModule shader_module{};
		VULKAN_CHECK(vkCreateShaderModule(m_Device.GetDevice(), &createInfo, nullptr, &shader_module), 
			"Failed to create ShaderModule ! ");

		return shader_module;
    }

    /*
	*
	*
	*
	*
	*
	*
	*
	------------------------------------------------------------------------
        Debug Drawing
    --------------------------------------------------------------------------
	*
	*
	*
	*
	*
	*
	*
	*/

    void RenderingServer::DrawPoint(glm::vec3 pos, glm::vec3 color) {
#if USE_DEBUG_DRAWING
		if(!ShowDD || !ShowDDPoints) { return; }
		m_DDCurrentFramePointsDrawCallHash += std::hash<glm::vec3>{}(pos) + std::hash<glm::vec3>{}(color);
		m_DDPoints.emplace_back(pos, color);
#endif
	}
	void RenderingServer::DrawLine(glm::vec3 startPos, glm::vec3 endPos, glm::vec3 color) {
#if USE_DEBUG_DRAWING
		if (!ShowDD || !ShowDDLines) { return; }
		m_DDCurrentFrameLinesDrawCallHash += std::hash<glm::vec3>{}(startPos) + std::hash<glm::vec3>{}(endPos) + std::hash<glm::vec3>{}(color);
		m_DDLines.emplace_back(startPos, color);
		m_DDLines.emplace_back(endPos, color);
#endif
	}
	void RenderingServer::DrawLineGradient(glm::vec3 startPos, glm::vec3 endPos, glm::vec3 startColor, glm::vec3 endColor) {
#if USE_DEBUG_DRAWING
		if (!ShowDD || !ShowDDLines) { return; }
		m_DDCurrentFrameLinesDrawCallHash += std::hash<glm::vec3>{}(startPos) + std::hash<glm::vec3>{}(endPos) + std::hash<glm::vec3>{}(startColor) + std::hash<glm::vec3>{}(endColor);
		m_DDLines.emplace_back(startPos, startColor);
		m_DDLines.emplace_back(endPos, endColor);
#endif
	}

	void RenderingServer::UpdateDebugDrawings() {
		// Recreate Buffers.
		//Points
		if(m_DDPoints.size() > 0 && m_DDCurrentFramePointsDrawCallHash != m_DDLastFramePointsDrawCallHash) {

			VkDeviceSize buffer_size = sizeof(m_DDPoints[0]) * m_DDPoints.size();

			VkBuffer staging_buffer;
			VkDeviceMemory staging_buffer_memory;

			CreateBuffer(m_Device.GetDevice(), m_Device.GetPhysicalDevice(), buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						 staging_buffer, staging_buffer_memory);

			void *data;
			vkMapMemory(m_Device.GetDevice(), staging_buffer_memory, 0, buffer_size, 0, &data);
			memcpy(data, m_DDPoints.data(), (size_t)buffer_size);
			vkUnmapMemory(m_Device.GetDevice(), staging_buffer_memory);

			CreateBuffer(m_Device.GetDevice(), m_Device.GetPhysicalDevice(), buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
						 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_DDPointsBuffer, m_DDPointsMemory);

			CopyBuffer(m_Device.GetDevice(), staging_buffer, m_DDPointsBuffer, buffer_size, m_CommandPool, m_Device.GetGraphicsQueue());

			vkDestroyBuffer(m_Device.GetDevice(), staging_buffer, nullptr);
			vkFreeMemory(m_Device.GetDevice(), staging_buffer_memory, nullptr);
		}

		m_DDPointsCount = m_DDPoints.size();
		m_DDPoints.clear();
		m_DDPoints.reserve(m_DDPointsCount);
		m_DDLastFramePointsDrawCallHash = m_DDCurrentFramePointsDrawCallHash;
		m_DDCurrentFramePointsDrawCallHash = 0;
		// Lines
		if(m_DDLines.size() > 0 && m_DDCurrentFrameLinesDrawCallHash != m_DDLastFrameLinesDrawCallHash) {

			VkDeviceSize buffer_size = sizeof(m_DDLines[0]) * m_DDLines.size();

			VkBuffer staging_buffer;
			VkDeviceMemory staging_buffer_memory;

			CreateBuffer(m_Device.GetDevice(), m_Device.GetPhysicalDevice(), buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						 staging_buffer, staging_buffer_memory);

			void *data;
			vkMapMemory(m_Device.GetDevice(), staging_buffer_memory, 0, buffer_size, 0, &data);
			memcpy(data, m_DDLines.data(), (size_t)buffer_size);
			vkUnmapMemory(m_Device.GetDevice(), staging_buffer_memory);

			CreateBuffer(m_Device.GetDevice(), m_Device.GetPhysicalDevice(), buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
						 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_DDLinesBuffer, m_DDLinesMemory);

			CopyBuffer(m_Device.GetDevice(), staging_buffer, m_DDLinesBuffer, buffer_size, m_CommandPool, m_Device.GetGraphicsQueue());

			vkDestroyBuffer(m_Device.GetDevice(), staging_buffer, nullptr);
			vkFreeMemory(m_Device.GetDevice(), staging_buffer_memory, nullptr);
		}
		m_DDLinesCount = m_DDLines.size();
		m_DDLines.clear();
		m_DDLines.reserve(m_DDLinesCount);
		m_DDLastFrameLinesDrawCallHash = m_DDCurrentFrameLinesDrawCallHash;
		m_DDCurrentFrameLinesDrawCallHash = 0;
	}
}