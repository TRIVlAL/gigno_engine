#include "swapchain.h"
#include <limits>
#include <algorithm>
#include "../core_macros.h"
#include "device.h"
#include "model.h"
#include <array>
#include <cstring>

#include "../core_macros.h"

#if USE_IMGUI
	#include "gui.h"
#endif

#include "rendering_utils.h"

#include "../entities/rendered_entity.h"
#include "../entities/camera.h"
#include "../entities/lights/light.h"
#include "rendering_server.h"

namespace gigno {

	giSwapChain::giSwapChain(const giDevice &device, const giWindow *window, const std::string &vertShaderPath, const std::string &fragShaderPath)
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

	void giSwapChain::CleanUp(VkDevice device) {
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

	void giSwapChain::Recreate(const giDevice &device, const giWindow *window, const std::string &vertShaderPath, const std::string &fragShaderPath) {
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
		
		CreateVkSwapChain(device.GetDevice(), device.GetPhysicalSwapChainSupport(), device.GetPhysicalDeviceQueueFamilyIndices(), device.GetSurface(), window, false);
		CreateImageViews(device.GetDevice());
		CreateDepthResources(device.GetDevice(), device.GetPhysicalDevice());
		CreateFrameBuffers(device.GetDevice());CreatePipelineLayout(device.GetDevice());
		CreatePipelineLayout(device.GetDevice());
		CreatePipeline(device.GetDevice(), vertShaderPath, fragShaderPath);
	}

	void giSwapChain::RecordCommandBuffer(uint32_t currentFrame, uint32_t imageIndex, const SceneRenderingData_t &sceneData) {
		ASSERT(currentFrame < MAX_FRAMES_IN_FLIGHT);
		RecordCommandBuffer(m_CommandBuffers[currentFrame], currentFrame, imageIndex, sceneData);
	}

	void giSwapChain::RecordCommandBuffer(VkCommandBuffer buffer, uint32_t currentFrame, uint32_t imageIndex, const SceneRenderingData_t &sceneData) {
		VkCommandBufferBeginInfo bufferBeginInfo{};
		bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		bufferBeginInfo.flags = 0;
		bufferBeginInfo.pInheritanceInfo = nullptr;

		VkResult result = vkBeginCommandBuffer(buffer, &bufferBeginInfo);
		if (result != VK_SUCCESS) {
			ERR_MSG("Failed to Begin Command Buffer ! Vulkan Error Code : " << (int)result);
		}

		std::array<VkClearValue, 2> clearValues = {};
		clearValues[0].color = { {0.15f, 0.15f, 0.15f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo passBeginInfo{};
		passBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		passBeginInfo.renderPass = m_RenderPass;
		passBeginInfo.framebuffer = m_FrameBuffers[imageIndex];
		passBeginInfo.renderArea.offset = { 0, 0 };
		passBeginInfo.renderArea.extent = m_Extent;
		passBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		passBeginInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(buffer, &passBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

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
			ERR_MSG("Failed to end Command Buffer ! Vulkan Error Code : " << (int)result);
		}
	}

	void giSwapChain::RenderEntities(VkCommandBuffer buffer, const std::vector<const RenderedEntity *> &entities, uint32_t currentFrame) {
		vkCmdSetPrimitiveTopology(buffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

		vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline.get()->GetVkPipeline());

		for (const RenderedEntity *entity : entities) {

			PushConstantData_t push{};
			push.modelMatrix = entity->Transform.TransformationMatrix();
			push.normalsMatrix = glm::mat4{entity->Transform.NormalMatrix()};
			push.fullbright = Fullbright;
			vkCmdPushConstants(buffer, m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantData_t), &push);

			entity->pModel->Bind(buffer);
			entity->pModel->Draw(buffer);
		}
	}

	
	#if USE_DEBUG_DRAWING
	void giSwapChain::UpdateDebugDrawings(VkDevice device, VkPhysicalDevice physDevice, VkQueue graphicsQueue) {
		// Recreate Buffers.
		//Points
		if(m_DDPoints.size() > 0 && m_DDCurrentFramePointsDrawCallHash != m_DDLastFramePointsDrawCallHash) {

			VkDeviceSize bufferSize = sizeof(m_DDPoints[0]) * m_DDPoints.size();

			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMemory;

			CreateBuffer(device, physDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						 stagingBuffer, stagingBufferMemory);

			void *data;
			vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
			memcpy(data, m_DDPoints.data(), (size_t)bufferSize);
			vkUnmapMemory(device, stagingBufferMemory);

			CreateBuffer(device, physDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
						VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_DDPointsBuffer, m_DDPointsMemory);

			CopyBuffer(device, stagingBuffer, m_DDPointsBuffer, bufferSize, m_CommandPool, graphicsQueue);

			vkDestroyBuffer(device, stagingBuffer, nullptr);
			vkFreeMemory(device, stagingBufferMemory, nullptr);

		}
		m_DDPointsCount = m_DDPoints.size();
		m_DDPoints.clear();
		m_DDPoints.reserve(m_DDPointsCount);
		m_DDLastFramePointsDrawCallHash = m_DDCurrentFramePointsDrawCallHash;
		m_DDCurrentFramePointsDrawCallHash = 0;
		// Lines
		if(m_DDLines.size() > 0 && m_DDCurrentFrameLinesDrawCallHash != m_DDLastFrameLinesDrawCallHash) {

			VkDeviceSize bufferSize = sizeof(m_DDLines[0]) * m_DDLines.size();

			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMemory;

			CreateBuffer(device, physDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						 stagingBuffer, stagingBufferMemory);

			void *data;
			vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
			memcpy(data, m_DDLines.data(), (size_t)bufferSize);
			vkUnmapMemory(device, stagingBufferMemory);

			CreateBuffer(device, physDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
						 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_DDLinesBuffer, m_DDLinesMemory);

			CopyBuffer(device, stagingBuffer, m_DDLinesBuffer, bufferSize, m_CommandPool, graphicsQueue);

			vkDestroyBuffer(device, stagingBuffer, nullptr);
			vkFreeMemory(device, stagingBufferMemory, nullptr);

		}
		m_DDLinesCount = m_DDLines.size();
		m_DDLines.clear();
		m_DDLines.reserve(m_DDLinesCount);
		m_DDLastFrameLinesDrawCallHash = m_DDCurrentFrameLinesDrawCallHash;
		m_DDCurrentFrameLinesDrawCallHash = 0;
	}

	void giSwapChain::DrawPoint(glm::vec3 position, glm::vec3 color, uint32_t drawCallHash) {
		m_DDCurrentFramePointsDrawCallHash += drawCallHash;
		m_DDPoints.emplace_back(Vertex{position, color});
	}

	void giSwapChain::DrawLine(glm::vec3 startPos, glm::vec3 endPos, glm::vec3 startColor, glm::vec3 endColor, uint32_t drawCallHash) {
		m_DDCurrentFrameLinesDrawCallHash += drawCallHash;
		m_DDLines.emplace_back(Vertex{startPos, startColor});
		m_DDLines.emplace_back(Vertex{endPos, endColor});
	}
	
	void giSwapChain::RenderDebugDrawings(VkCommandBuffer buffer, const Camera *camera, uint32_t currentFrame) {
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

	void giSwapChain::UpdateUniformBuffer(VkCommandBuffer commandBuffer, const Camera *camera, const std::vector<const Light *> &lights, uint32_t currentFrame)
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

	void giSwapChain::CreateUniformBuffers(VkDevice device, VkPhysicalDevice physDevice) {
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

	void giSwapChain::CreateDescriptorPool(VkDevice device) {
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
			ERR_MSG("Failed to create Descriptor Pool ! Vulkan Error Code : " << (int)result);
		}
	}

	void giSwapChain::CreateDescriptorSets(VkDevice device) {
		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_DescriptorSetLayout);
		VkDescriptorSetAllocateInfo allocinfo{};
		allocinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocinfo.descriptorPool = m_DescriptorPool;
		allocinfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
		allocinfo.pSetLayouts = layouts.data();

		m_DescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
		VkResult result = vkAllocateDescriptorSets(device, &allocinfo, m_DescriptorSets.data());
		if(result != VK_SUCCESS) {
			ERR_MSG("Failed to Allocate Descriptor Sets ! Vulkan Error Codes : " << (int)result);
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

	void giSwapChain::CreateDescriptorSetLayout(VkDevice device) {
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.bindingCount = 1;
		createInfo.pBindings = &uboLayoutBinding;

		VkResult result = vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &m_DescriptorSetLayout);
		if(result != VK_SUCCESS) {
			ERR_MSG("Failed to create Descriptor Set Layout ! Vulkan Error Code : " << (int)result);
		}
	}

	void giSwapChain::CreatePipelineLayout(VkDevice device) {
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(PushConstantData_t);

		VkPipelineLayoutCreateInfo createInfo;
		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.setLayoutCount = 1;
		createInfo.pSetLayouts = &m_DescriptorSetLayout;
		createInfo.pushConstantRangeCount = 1;
		createInfo.pPushConstantRanges = &pushConstantRange;

		VkResult result = vkCreatePipelineLayout(device, &createInfo, nullptr, &m_PipelineLayout);
		if (result != VK_SUCCESS) {
			ERR_MSG("Failed to create Vulkan Pipeline Layout ! Vulkan Error Code : " << (int)result);
		}
	}

	void giSwapChain::CreatePipeline(VkDevice device, const std::string &vertShaderFilePath, const std::string &fragShaderFilePath) {
		giPipelineConfigInfo info{};
		giPipeline::DefaultConfig(info);
		info.renderPass = m_RenderPass;
		info.pipelineLayout = m_PipelineLayout;
		m_Pipeline = std::make_unique<giPipeline>(device, vertShaderFilePath, fragShaderFilePath, info);
	}

	void giSwapChain::CreateVkSwapChain(VkDevice device, const SwapChainSupportDetails &supportDetails, const QueueFamilyIndices &physicalDeviceQueueFamilyIndices, VkSurfaceKHR surface, const giWindow *window, bool isFirstCreation) {
		VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(supportDetails.formats);
		VkPresentModeKHR presentMode = ChooseSwapPresentMode(supportDetails.presentModes);
		VkExtent2D extent = ChooseSwapExtent(window, supportDetails.surfaceCapabilities);

		m_Extent = extent;
		m_Format = surfaceFormat.format;

		uint32_t imageCount = supportDetails.surfaceCapabilities.minImageCount + 1;
		if (supportDetails.surfaceCapabilities.maxImageCount > 0 && imageCount > supportDetails.surfaceCapabilities.maxImageCount) {
			imageCount = supportDetails.surfaceCapabilities.maxImageCount;
		}

		uint32_t queueFamilyIndices[] = { physicalDeviceQueueFamilyIndices.graphicFamily.value(), physicalDeviceQueueFamilyIndices.presentFamily.value() };

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.pNext = nullptr;
		createInfo.surface = surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		if (physicalDeviceQueueFamilyIndices.graphicFamily != physicalDeviceQueueFamilyIndices.presentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}
		createInfo.preTransform = supportDetails.surfaceCapabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = isFirstCreation ? VK_NULL_HANDLE : m_VkSwapChain;

		VkResult result = vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_VkSwapChain);
		if (result != VK_SUCCESS) {
			ERR_MSG("Failed to create Vulkan Swapchain ! Vulkan error code : " << (int)result);
		}

		uint32_t imagesCount = 0;
		vkGetSwapchainImagesKHR(device, m_VkSwapChain, &imagesCount, nullptr);
		m_Images.resize(imagesCount);

		vkGetSwapchainImagesKHR(device, m_VkSwapChain, &imagesCount, m_Images.data());
	}

	void giSwapChain::CreateImageViews(VkDevice device) {
		m_ImageViews.resize(m_Images.size());
		for (size_t i = 0; i < m_Images.size(); i++) {
			m_ImageViews[i] = CreateImageView(device, m_Images[i], m_Format, VK_IMAGE_ASPECT_COLOR_BIT);
		}
	}

	void giSwapChain::CreateRenderPass(const VkDevice &device, const VkPhysicalDevice &physDevice) {
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = m_Format;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = FindDepthFormat(physDevice);
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		
		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 2> attachments{ colorAttachment, depthAttachment };
		VkRenderPassCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		createInfo.pAttachments = attachments.data();
		createInfo.subpassCount = 1;
		createInfo.pSubpasses = &subpass;
		createInfo.dependencyCount = 1;
		createInfo.pDependencies = &dependency;
		
		VkResult result = vkCreateRenderPass(device, &createInfo, nullptr, &m_RenderPass);
		if (result != VK_SUCCESS) {
			ERR_MSG("Failed to create Render Pass ! Vulkan Error Code : " << (int)result);
		}
	}

	void giSwapChain::CreateFrameBuffers(VkDevice device) {
		m_FrameBuffers.resize(m_ImageViews.size());
		for (size_t i = 0; i < m_ImageViews.size(); i++) {
			std::array<VkImageView, 2> attachments = { m_ImageViews[i], m_DepthImageView };

			VkFramebufferCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			createInfo.renderPass = m_RenderPass;
			createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			createInfo.pAttachments = attachments.data();
			createInfo.width = m_Extent.width;
			createInfo.height = m_Extent.height;
			createInfo.layers = 1;

			VkResult result = vkCreateFramebuffer(device, &createInfo, nullptr, &m_FrameBuffers[i]);
			if (result != VK_SUCCESS) {
				ERR_MSG("Failed to create Vulkan Frame Buffer #" << i << ". Vulkan Error Code : " << (int)result);
			}
		}
	}

	void giSwapChain::CreateCommandPool(VkDevice device, QueueFamilyIndices indices) {
		VkCommandPoolCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		createInfo.queueFamilyIndex = indices.graphicFamily.value();

		VkResult result = vkCreateCommandPool(device, &createInfo, nullptr, &m_CommandPool);
		if (result != VK_SUCCESS) {
			ERR_MSG("Failed to create Vulkan Command Pool. Vulkan Error Code : " << (int)result);
		}
	}

	void giSwapChain::CreateDepthResources(VkDevice device, VkPhysicalDevice physDevice) {
		VkFormat format = FindDepthFormat(physDevice);
		CreateImage(device, physDevice, m_Extent.width, m_Extent.height, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_DepthImage, m_DepthImageMemory);
		m_DepthImageView = CreateImageView(device, m_DepthImage, format, VK_IMAGE_ASPECT_DEPTH_BIT);
	}

	void giSwapChain::CreateCommandBuffers(VkDevice device) {
		m_CommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)m_CommandBuffers.size();

		VkResult result = vkAllocateCommandBuffers(device, &allocInfo, m_CommandBuffers.data());
		if (result != VK_SUCCESS) {
			ERR_MSG("Failed to allocate Vulkan Command Buffers. Vulkan Error Code : " << (int)result);
		}
	}

	VkSurfaceFormatKHR giSwapChain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &avaliableFormats) {
		for (const VkSurfaceFormatKHR &format : avaliableFormats) {
			if (format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && format.format == VK_FORMAT_B8G8R8A8_UNORM) {
				return format;
			}
		}
		return avaliableFormats[0];
	}

	VkPresentModeKHR giSwapChain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &avaliableModes) {
		for (const VkPresentModeKHR &mode : avaliableModes) {
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return mode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D giSwapChain::ChooseSwapExtent(const giWindow * window, const VkSurfaceCapabilitiesKHR &capabilities) {
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		}
		else {
			int width;
			int height;
			window->GetFrameBufferSize(&width, &height);

			VkExtent2D actualExtent{
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}

	VkFormat giSwapChain::FindSupportedFormat(VkPhysicalDevice physDevice, const std::vector<VkFormat> &candidates, VkImageTiling targetTiling, VkFormatFeatureFlags targetFeatures) {
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
		ERR_MSG_V("Failed to find supported format !", VkFormat{});
	}

	VkFormat giSwapChain::FindDepthFormat(VkPhysicalDevice physDevice) {
		return FindSupportedFormat(physDevice, { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}

	uint32_t giSwapChain::FindMemoryTypeIndex(VkPhysicalDevice device, uint32_t typeFilter, VkMemoryPropertyFlags properties) const {
		VkPhysicalDeviceMemoryProperties memProp{};
		vkGetPhysicalDeviceMemoryProperties(device, &memProp);

		for (uint32_t i = 0; i < memProp.memoryTypeCount; i++) {
			if ((typeFilter & (i << i)) && (memProp.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		ERR_MSG_V("Failed to find suitable memory type !", UINT32_MAX);
	}

	void giSwapChain::CreateImage(VkDevice device, VkPhysicalDevice physDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
					VkMemoryPropertyFlags props, VkImage &image, VkDeviceMemory &imageMemory) {
		VkImageCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		createInfo.imageType = VK_IMAGE_TYPE_2D;
		createInfo.format = format;
		createInfo.extent.width = width;
		createInfo.extent.height = height;
		createInfo.extent.depth = 1;
		createInfo.mipLevels = 1;
		createInfo.arrayLayers = 1;
		createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		createInfo.tiling = tiling;
		createInfo.usage = usage;
		createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;


		VkResult result = vkCreateImage(device, &createInfo, nullptr, &image);
		if (result != VK_SUCCESS) {
			ERR_MSG("Failed to create Vulkan Image ! Vulkan Error Code : " << (int)result);
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryTypeIndex(physDevice, memRequirements.memoryTypeBits, props);

		result = vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory);
		if (result != VK_SUCCESS) {
			ERR_MSG("Failed to allocate Image Memory ! Vulkan Error Code : " << (int)result);
		}

		vkBindImageMemory(device, image, imageMemory, 0);
	}

	VkImageView giSwapChain::CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.image = image;
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = format;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = aspectFlags;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		VkImageView ret{};
		VkResult result = vkCreateImageView(device, &createInfo, nullptr, &ret);
		if (result != VK_SUCCESS) {
			ERR_MSG_V("Failed to create Vulkan Image View ! Vulkan Error Code : " << (int)result, ret);
		}
		return ret;
	}


}
