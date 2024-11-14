#include "swapchain.h"
#include <limits>
#include <algorithm>
#include "../features_usage.h"
#include "device.h"
#include "model.h"
#include <array>
#include <cstring>

#include "../application.h"

#include "../error_macros.h"

#if USE_IMGUI
	#include "gui.h"
#endif

#include "rendering_utils.h"

#include "../entities/rendered_entity.h"
#include "../entities/camera.h"
#include "../entities/lights/light.h"
#include "rendering_server.h"

namespace gigno {

	SwapChain::SwapChain(const Device &device, const Window *window, const std::string &vertShaderPath, const std::string &fragShaderPath)
	{
		CreateVkSwapChain(device.GetDevice(), device.GetPhysicalSwapChainSupport(), device.GetPhysicalDeviceQueueFamilyIndices(), device.GetSurface(), window, true);
		CreateImageViews(device.GetDevice());
		CreateRenderPass(device.GetDevice(), device.GetPhysicalDevice());
		CreateCommandPool(device.GetDevice(), device.GetPhysicalDeviceQueueFamilyIndices());
		CreateDepthResources(device.GetDevice(), device.GetPhysicalDevice());
		CreateDescriptorSetLayout(device.GetDevice());
		CreateUniformBuffers(device.GetDevice(), device.GetPhysicalDevice());
		CreateDescriptorPool(device.GetDevice());
		CreateDescriptorSets(device.GetDevice());
		CreateFrameBuffers(device.GetDevice());
		CreateCommandBuffers(device.GetDevice());
		CreatePipelineLayout(device.GetDevice());
		CreatePipeline(device.GetDevice(), vertShaderPath, fragShaderPath);
	}

	void SwapChain::CleanUp(VkDevice device) {
		for (auto imageView : m_ImageViews) {
			vkDestroyImageView(device, imageView, nullptr);
		}
		for (auto frameBuffer : m_FrameBuffers) {
			vkDestroyFramebuffer(device, frameBuffer, nullptr);
		}
		vkDestroyImageView(device, m_DepthImageView, nullptr);
		vkDestroyImage(device, m_DepthImage, nullptr);
		vkFreeMemory(device, m_DepthImageMemory, nullptr);

		vkDestroyPipelineLayout(device, m_PipelineLayout, nullptr);
		vkDestroyCommandPool(device, m_CommandPool, nullptr);
		vkDestroySwapchainKHR(device, m_VkSwapChain, nullptr);
		vkDestroyRenderPass(device, m_RenderPass, nullptr);

		for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroyBuffer(device, m_UniformBuffers[i], nullptr);
			vkFreeMemory(device, m_UniformBuffersMemories[i], nullptr);
		}
		vkDestroyDescriptorPool(device, m_DescriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(device, m_DescriptorSetLayout, nullptr);
	}

	void SwapChain::Recreate(const Device &device, const Window *window, const std::string &vertShaderPath, const std::string &fragShaderPath) {
		int width = 0, height = 0;
		window->GetFrameBufferSize(&width, &height);
		while (width == 0 || height == 0) {
			window->GetFrameBufferSize(&width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(device.GetDevice());

		for (auto imageView : m_ImageViews) {
			vkDestroyImageView(device.GetDevice(), imageView, nullptr);
		}
		for (auto frameBuffer : m_FrameBuffers) {
			vkDestroyFramebuffer(device.GetDevice(), frameBuffer, nullptr);
		}

		vkDestroyImageView(device.GetDevice(), m_DepthImageView, nullptr);
		vkDestroyImage(device.GetDevice(), m_DepthImage, nullptr);
		vkFreeMemory(device.GetDevice(), m_DepthImageMemory, nullptr);
		vkDestroyPipelineLayout(device.GetDevice(), m_PipelineLayout, nullptr);
		
		CreateVkSwapChain(device.GetDevice(), device.GetPhysicalSwapChainSupport(), device.GetPhysicalDeviceQueueFamilyIndices(), device.GetSurface(), window, false);
		CreateImageViews(device.GetDevice());
		CreateDepthResources(device.GetDevice(), device.GetPhysicalDevice());
		CreateFrameBuffers(device.GetDevice());
		CreatePipelineLayout(device.GetDevice());
		m_Pipeline.reset();
		CreatePipeline(device.GetDevice(), vertShaderPath, fragShaderPath);
	}

	void SwapChain::RecordCommandBuffer(uint32_t currentFrame, uint32_t imageIndex, const SceneRenderingData_t &sceneData) {
		ASSERT(currentFrame < MAX_FRAMES_IN_FLIGHT);
		RecordCommandBuffer(m_CommandBuffers[currentFrame], currentFrame, imageIndex, sceneData);
	}

	void SwapChain::RecordCommandBuffer(VkCommandBuffer buffer, uint32_t currentFrame, uint32_t imageIndex, const SceneRenderingData_t &sceneData) {
		VkCommandBufferBeginInfo buffer_begin_info{};
		buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		buffer_begin_info.flags = 0;
		buffer_begin_info.pInheritanceInfo = nullptr;

		VkResult result = vkBeginCommandBuffer(buffer, &buffer_begin_info);
		if (result != VK_SUCCESS) {
			ERR_MSG("Failed to Begin Command Buffer ! Vulkan Error Code : %d", (int)result);
		}

		std::array<VkClearValue, 2> clear_vals = {};
		clear_vals[0].color = {{.15f, 0.15f, 0.15f, 1.0f}};
		clear_vals[1].depthStencil = {1.0f, 0};

		VkRenderPassBeginInfo pass_begin_info{};
		pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		pass_begin_info.renderPass = m_RenderPass;
		pass_begin_info.framebuffer = m_FrameBuffers[imageIndex];
		pass_begin_info.renderArea.offset = {0, 0};
		pass_begin_info.renderArea.extent = m_Extent;
		pass_begin_info.clearValueCount = static_cast<uint32_t>(clear_vals.size());
		pass_begin_info.pClearValues = clear_vals.data();

		vkCmdBeginRenderPass(buffer, &pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_Extent.width);
		viewport.height = static_cast<float>(m_Extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(buffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.extent = m_Extent;
		scissor.offset = { 0, 0 };
		vkCmdSetScissor(buffer, 0, 1, &scissor);

		if (sceneData.pCamera) {
			UpdateUniformBuffer(buffer, sceneData.pCamera, sceneData.LightEntities, currentFrame);
			RenderEntities(buffer, sceneData.RenderedEntities, currentFrame);
			#if USE_DEBUG_DRAWING
			RenderDebugDrawings(buffer, sceneData.pCamera, currentFrame);
			#endif
		}


		#if USE_IMGUI
		RenderImGui(buffer);
		#endif

		vkCmdEndRenderPass(buffer);

		result = vkEndCommandBuffer(buffer);
		if (result != VK_SUCCESS) {
			ERR_MSG("Failed to end Command Buffer ! Vulkan Error Code : %d", (int)result);
		}
	}

	void SwapChain::RenderEntities(VkCommandBuffer buffer, const RenderedEntity * entities, uint32_t currentFrame) {
		vkCmdSetPrimitiveTopology(buffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

		vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline.get()->GetVkPipeline());

		const RenderedEntity *curr = entities;
		while(curr) {
			PushConstantData_t push{};
			push.modelMatrix = curr->Transform.TransformationMatrix();
			push.normalsMatrix = glm::mat4{curr->Transform.NormalMatrix()};
			push.fullbright = Fullbright;
			vkCmdPushConstants(buffer, m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantData_t), &push);

			curr->pModel->Bind(buffer);
			curr->pModel->Draw(buffer);

			curr = curr->pNextRenderedEntity;
		}
	}

	
	#if USE_DEBUG_DRAWING
	void SwapChain::UpdateDebugDrawings(VkDevice device, VkPhysicalDevice physDevice, VkQueue graphicsQueue) {
		// Recreate Buffers.
		//Points
		if(m_DDPoints.size() > 0 && m_DDCurrentFramePointsDrawCallHash != m_DDLastFramePointsDrawCallHash) {

			VkDeviceSize buffer_size = sizeof(m_DDPoints[0]) * m_DDPoints.size();

			VkBuffer staging_buffer;
			VkDeviceMemory staging_buffer_memory;

			CreateBuffer(device, physDevice, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						 staging_buffer, staging_buffer_memory);

			void *data;
			vkMapMemory(device, staging_buffer_memory, 0, buffer_size, 0, &data);
			memcpy(data, m_DDPoints.data(), (size_t)buffer_size);
			vkUnmapMemory(device, staging_buffer_memory);

			CreateBuffer(device, physDevice, buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
						 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_DDPointsBuffer, m_DDPointsMemory);

			CopyBuffer(device, staging_buffer, m_DDPointsBuffer, buffer_size, m_CommandPool, graphicsQueue);

			vkDestroyBuffer(device, staging_buffer, nullptr);
			vkFreeMemory(device, staging_buffer_memory, nullptr);
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

			CreateBuffer(device, physDevice, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						 staging_buffer, staging_buffer_memory);

			void *data;
			vkMapMemory(device, staging_buffer_memory, 0, buffer_size, 0, &data);
			memcpy(data, m_DDLines.data(), (size_t)buffer_size);
			vkUnmapMemory(device, staging_buffer_memory);

			CreateBuffer(device, physDevice, buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
						 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_DDLinesBuffer, m_DDLinesMemory);

			CopyBuffer(device, staging_buffer, m_DDLinesBuffer, buffer_size, m_CommandPool, graphicsQueue);

			vkDestroyBuffer(device, staging_buffer, nullptr);
			vkFreeMemory(device, staging_buffer_memory, nullptr);
		}
		m_DDLinesCount = m_DDLines.size();
		m_DDLines.clear();
		m_DDLines.reserve(m_DDLinesCount);
		m_DDLastFrameLinesDrawCallHash = m_DDCurrentFrameLinesDrawCallHash;
		m_DDCurrentFrameLinesDrawCallHash = 0;
	}

	void SwapChain::DrawPoint(glm::vec3 position, glm::vec3 color, uint32_t drawCallHash) {
		m_DDCurrentFramePointsDrawCallHash += drawCallHash;
		m_DDPoints.emplace_back(position, color);
	}

	void SwapChain::DrawLine(glm::vec3 startPos, glm::vec3 endPos, glm::vec3 startColor, glm::vec3 endColor, uint32_t drawCallHash) {
		m_DDCurrentFrameLinesDrawCallHash += drawCallHash;
		m_DDLines.emplace_back(startPos, startColor);
		m_DDLines.emplace_back(endPos, endColor);
	}
	
	void SwapChain::RenderDebugDrawings(VkCommandBuffer buffer, const Camera *camera, uint32_t currentFrame) {
		PushConstantData_t push{};
		push.modelMatrix = {1.0f};
		push.normalsMatrix = {1.0f};
		push.fullbright = true;
		vkCmdPushConstants(buffer, m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantData_t), &push);

		VkDeviceSize offset = 0;
		
		if(m_DDPointsCount > 0) {
			vkCmdSetPrimitiveTopology(buffer, VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
			vkCmdBindVertexBuffers(buffer, 0, 1, &m_DDPointsBuffer, &offset);
			vkCmdDraw(buffer, m_DDPointsCount, 1, 0, 0);
		}

		if(m_DDLinesCount > 0) {
		vkCmdSetPrimitiveTopology(buffer, VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
		vkCmdBindVertexBuffers(buffer, 0, 1, &m_DDLinesBuffer, &offset);
		vkCmdDraw(buffer, m_DDLinesCount, 1, 0, 0);
		}
	}
	#endif

	void SwapChain::UpdateUniformBuffer(VkCommandBuffer commandBuffer, const Camera *camera, const std::vector<const Light *> &lights, uint32_t currentFrame)
	{
		ASSERT(currentFrame < MAX_FRAMES_IN_FLIGHT);

		const glm::mat4 proj = camera->GetProjection();
		const glm::mat4 view = camera->GetViewMatrix();

		UniformBufferData_t ub{};
		ub.projection = proj;
		ub.view = view;

		uint32_t i = 0;
		for(const Light *light : lights) {
			const uint32_t advance = light->DataSlotsCount();
			if(i + advance > MAX_LIGHT_DATA_COUNT) {
				break;
			}
			light->FillDataSlots(&ub.lightDatas[i]);
			i += advance;
		}

		std::memcpy(m_UniformBuffersMapped[currentFrame], &ub, sizeof(ub));

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_DescriptorSets[currentFrame], 0, nullptr);
	}

	void SwapChain::CreateUniformBuffers(VkDevice device, VkPhysicalDevice physDevice) {
		VkDeviceSize size = sizeof(UniformBufferData_t);

		m_UniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		m_UniformBuffersMemories.resize(MAX_FRAMES_IN_FLIGHT);
		m_UniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

		for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			CreateBuffer(device, physDevice, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
									m_UniformBuffers[i], m_UniformBuffersMemories[i]);
			vkMapMemory(device, m_UniformBuffersMemories[i], 0, size, 0, &m_UniformBuffersMapped[i]);
		}
	}

	void SwapChain::CreateDescriptorPool(VkDevice device) {
		VkDescriptorPoolSize size{};
		size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		size.descriptorCount = MAX_FRAMES_IN_FLIGHT;

		VkDescriptorPoolCreateInfo createinfo{};
		createinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		createinfo.maxSets = MAX_FRAMES_IN_FLIGHT;
		createinfo.poolSizeCount = 1;
		createinfo.pPoolSizes = &size;

		VkResult result = vkCreateDescriptorPool(device, &createinfo, nullptr, &m_DescriptorPool);
		if(result != VK_SUCCESS) {
			ERR_MSG("Failed to create Descriptor Pool ! Vulkan Errror Code : %d", (int)result);
		}
	}

	void SwapChain::CreateDescriptorSets(VkDevice device) {
		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_DescriptorSetLayout);
		VkDescriptorSetAllocateInfo allocinfo{};
		allocinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocinfo.descriptorPool = m_DescriptorPool;
		allocinfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
		allocinfo.pSetLayouts = layouts.data();

		m_DescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
		VkResult result = vkAllocateDescriptorSets(device, &allocinfo, m_DescriptorSets.data());
		if(result != VK_SUCCESS) {
			ERR_MSG("Failed to Allocate Descriptor Sets ! Vulkan Error Code : %d", (int)result);
		}

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

			vkUpdateDescriptorSets(device, 1, &descWrite, 0, nullptr);
		}
	}

	void SwapChain::CreateDescriptorSetLayout(VkDevice device) {
		VkDescriptorSetLayoutBinding ubo_layout_binding{};
		ubo_layout_binding.binding = 0;
		ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		ubo_layout_binding.descriptorCount = 1;
		ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutCreateInfo createinfo{};
		createinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createinfo.pNext = nullptr;
		createinfo.flags = 0;
		createinfo.bindingCount = 1;
		createinfo.pBindings = &ubo_layout_binding;

		VkResult result = vkCreateDescriptorSetLayout(device, &createinfo, nullptr, &m_DescriptorSetLayout);
		if(result != VK_SUCCESS) {
			ERR_MSG("Failed to create Descriptor Set Layout ! Vulkan Errror Code : %d", (int)result);
		}
	}

	void SwapChain::CreatePipelineLayout(VkDevice device) {
		VkPushConstantRange push_constant_range{};
		push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		push_constant_range.offset = 0;
		push_constant_range.size = sizeof(PushConstantData_t);

		VkPipelineLayoutCreateInfo createinfo;
		createinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		createinfo.pNext = nullptr;
		createinfo.setLayoutCount = 1;
		createinfo.pSetLayouts = &m_DescriptorSetLayout;
		createinfo.pushConstantRangeCount = 1;
		createinfo.pPushConstantRanges = &push_constant_range;

		VkResult result = vkCreatePipelineLayout(device, &createinfo, nullptr, &m_PipelineLayout);
		if (result != VK_SUCCESS) {
			ERR_MSG("Failed to create Vulkan Pipeline Layout ! Vulkan Errror Code : %d", (int)result);
		}
	}

	void SwapChain::CreatePipeline(VkDevice device, const std::string &vertShaderFilePath, const std::string &fragShaderFilePath) {
		giPipelineConfigInfo info{};
		giPipeline::DefaultConfig(info);
		info.renderPass = m_RenderPass;
		info.pipelineLayout = m_PipelineLayout;
		m_Pipeline = std::make_unique<giPipeline>(device, vertShaderFilePath, fragShaderFilePath, info);
	}

	void SwapChain::CreateVkSwapChain(VkDevice device, const SwapChainSupportDetails &supportDetails, const QueueFamilyIndices &physicalDeviceQueueFamilyIndices, VkSurfaceKHR surface, const Window *window, bool isFirstCreation) {
		VkSurfaceFormatKHR surface_fmt = ChooseSwapSurfaceFormat(supportDetails.formats);
		VkPresentModeKHR present_mode = ChooseSwapPresentMode(supportDetails.presentModes);
		VkExtent2D extent = ChooseSwapExtent(window, supportDetails.surfaceCapabilities);

		m_Extent = extent;
		m_Format = surface_fmt.format;

		uint32_t imageCount = supportDetails.surfaceCapabilities.minImageCount + 1;
		if (supportDetails.surfaceCapabilities.maxImageCount > 0 && imageCount > supportDetails.surfaceCapabilities.maxImageCount) {
			imageCount = supportDetails.surfaceCapabilities.maxImageCount;
		}

		uint32_t queue_fam_indices[] = { physicalDeviceQueueFamilyIndices.graphicFamily.value(), physicalDeviceQueueFamilyIndices.presentFamily.value() };

		VkSwapchainKHR existing_swapchain = isFirstCreation ? VK_NULL_HANDLE : m_VkSwapChain;

		VkSwapchainCreateInfoKHR createinfo{};
		createinfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createinfo.pNext = nullptr;
		createinfo.surface = surface;
		createinfo.minImageCount = imageCount;
		createinfo.imageFormat = surface_fmt.format;
		createinfo.imageColorSpace = surface_fmt.colorSpace;
		createinfo.imageExtent = extent;
		createinfo.imageArrayLayers = 1;
		createinfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		if (physicalDeviceQueueFamilyIndices.graphicFamily != physicalDeviceQueueFamilyIndices.presentFamily) {
			createinfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createinfo.queueFamilyIndexCount = 2;
			createinfo.pQueueFamilyIndices = queue_fam_indices;
		}
		else {
			createinfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createinfo.queueFamilyIndexCount = 0;
			createinfo.pQueueFamilyIndices = nullptr;
		}
		createinfo.preTransform = supportDetails.surfaceCapabilities.currentTransform;
		createinfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createinfo.presentMode = present_mode;
		createinfo.clipped = VK_TRUE;
		createinfo.oldSwapchain = existing_swapchain;

		VkResult result = vkCreateSwapchainKHR(device, &createinfo, nullptr, &m_VkSwapChain);
		if (result != VK_SUCCESS) {
			ERR_MSG("Failed to create Vulkan Swapchain ! Vulkan Errror Code : %d", (int)result);
		}

		if(existing_swapchain != VK_NULL_HANDLE) {
			vkDestroySwapchainKHR(device, existing_swapchain, nullptr);
		}

		uint32_t images_count = 0;
		vkGetSwapchainImagesKHR(device, m_VkSwapChain, &images_count, nullptr);
		m_Images.resize(images_count);

		vkGetSwapchainImagesKHR(device, m_VkSwapChain, &images_count, m_Images.data());
	}

	void SwapChain::CreateImageViews(VkDevice device) {
		m_ImageViews.resize(m_Images.size());
		for (size_t i = 0; i < m_Images.size(); i++) {
			m_ImageViews[i] = CreateImageView(device, m_Images[i], m_Format, VK_IMAGE_ASPECT_COLOR_BIT);
		}
	}

	void SwapChain::CreateRenderPass(const VkDevice &device, const VkPhysicalDevice &physDevice) {
		VkAttachmentDescription color_attachment{};
		color_attachment.format = m_Format;
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
		depth_attachment.format = FindDepthFormat(physDevice);
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
		
		VkResult result = vkCreateRenderPass(device, &createinfo, nullptr, &m_RenderPass);
		if (result != VK_SUCCESS) {
			ERR_MSG("Failed to create Render Pass ! Vulkan Errror Code : %d", (int)result);
		}
	}

	void SwapChain::CreateFrameBuffers(VkDevice device) {
		m_FrameBuffers.resize(m_ImageViews.size());
		for (size_t i = 0; i < m_ImageViews.size(); i++) {
			std::array<VkImageView, 2> attachments = { m_ImageViews[i], m_DepthImageView };

			VkFramebufferCreateInfo createinfo{};
			createinfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			createinfo.renderPass = m_RenderPass;
			createinfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			createinfo.pAttachments = attachments.data();
			createinfo.width = m_Extent.width;
			createinfo.height = m_Extent.height;
			createinfo.layers = 1;

			VkResult result = vkCreateFramebuffer(device, &createinfo, nullptr, &m_FrameBuffers[i]);
			if (result != VK_SUCCESS) {
				ERR_MSG("Failed to create Vulkan Frame Buffer #%d. Vulkan Errror Code : %d", i, (int)result);
			}
		}
	}

	void SwapChain::CreateCommandPool(VkDevice device, QueueFamilyIndices indices) {
		VkCommandPoolCreateInfo createinfo{};
		createinfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createinfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		createinfo.queueFamilyIndex = indices.graphicFamily.value();

		VkResult result = vkCreateCommandPool(device, &createinfo, nullptr, &m_CommandPool);
		if (result != VK_SUCCESS) {
			ERR_MSG("Failed to create Vulkan Command Pool. Vulkan Errror Code : %d", (int)result);
		}
	}

	void SwapChain::CreateDepthResources(VkDevice device, VkPhysicalDevice physDevice) {
		VkFormat format = FindDepthFormat(physDevice);
		CreateImage(device, physDevice, m_Extent.width, m_Extent.height, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_DepthImage, m_DepthImageMemory);
		m_DepthImageView = CreateImageView(device, m_DepthImage, format, VK_IMAGE_ASPECT_DEPTH_BIT);
	}

	void SwapChain::CreateCommandBuffers(VkDevice device) {
		m_CommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocinfo{};
		allocinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocinfo.commandPool = m_CommandPool;
		allocinfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocinfo.commandBufferCount = (uint32_t)m_CommandBuffers.size();

		VkResult result = vkAllocateCommandBuffers(device, &allocinfo, m_CommandBuffers.data());
		if (result != VK_SUCCESS) {
			ERR_MSG("Failed to allocate Vulkan Command Buffers. Vulkan Errror Code : %d", (int)result);
		}
	}

	VkSurfaceFormatKHR SwapChain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &avaliableFormats) {
		for (const VkSurfaceFormatKHR &format : avaliableFormats) {
			if (format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && format.format == VK_FORMAT_B8G8R8A8_UNORM) {
				return format;
			}
		}
		return avaliableFormats[0];
	}

	VkPresentModeKHR SwapChain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &avaliableModes) {
		for (const VkPresentModeKHR &mode : avaliableModes) {
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return mode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D SwapChain::ChooseSwapExtent(const Window * window, const VkSurfaceCapabilitiesKHR &capabilities) {
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

	VkFormat SwapChain::FindSupportedFormat(VkPhysicalDevice physDevice, const std::vector<VkFormat> &candidates, VkImageTiling targetTiling, VkFormatFeatureFlags targetFeatures) {
		for (VkFormat format : candidates) {
			VkFormatProperties prop;
			vkGetPhysicalDeviceFormatProperties(physDevice, format, &prop);
			if (targetTiling == VK_IMAGE_TILING_LINEAR && (prop.linearTilingFeatures & targetFeatures) == targetFeatures) {
				return format;
			}
			else if(targetTiling == VK_IMAGE_TILING_OPTIMAL && (prop.optimalTilingFeatures & targetFeatures) == targetFeatures) {
				return format;
			}
		}
		ERR_MSG_V(VkFormat{}, "Failed to find supported format !");
	}

	VkFormat SwapChain::FindDepthFormat(VkPhysicalDevice physDevice) {
		return FindSupportedFormat(physDevice, { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}

	uint32_t SwapChain::FindMemoryTypeIndex(VkPhysicalDevice device, uint32_t typeFilter, VkMemoryPropertyFlags properties) const {
		VkPhysicalDeviceMemoryProperties mem_prop{};
		vkGetPhysicalDeviceMemoryProperties(device, &mem_prop);

		for (uint32_t i = 0; i < mem_prop.memoryTypeCount; i++) {
			if ((typeFilter & (i << i)) && (mem_prop.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		ERR_MSG_V(UINT32_MAX, "Failed to find suitable memory type !");
	}

	void SwapChain::CreateImage(VkDevice device, VkPhysicalDevice physDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
					VkMemoryPropertyFlags props, VkImage &image, VkDeviceMemory &imageMemory) {
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
			ERR_MSG("Failed to create Vulkan Image ! Vulkan Errror Code : %d", (int)result);
		}

		VkMemoryRequirements mem_requirements;
		vkGetImageMemoryRequirements(device, image, &mem_requirements);

		VkMemoryAllocateInfo allocinfo{};
		allocinfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocinfo.allocationSize = mem_requirements.size;
		allocinfo.memoryTypeIndex = FindMemoryTypeIndex(physDevice, mem_requirements.memoryTypeBits, props);

		result = vkAllocateMemory(device, &allocinfo, nullptr, &imageMemory);
		if (result != VK_SUCCESS) {
			ERR_MSG("Failed to allocate Image Memory ! Vulkan Errror Code : %d", (int)result);
		}

		vkBindImageMemory(device, image, imageMemory, 0);
	}

	VkImageView SwapChain::CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
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
			ERR_MSG_V(ret, "Failed to create Vulkan Image View ! Vulkan Errror Code : %d", (int)result);
		}
		return ret;
	}


}
