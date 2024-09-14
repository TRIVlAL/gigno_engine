#include "swapchain.h"
#include <limits>
#include <algorithm>
#include "../core_macros.h"
#include "device.h"
#include "model.h"
#include <array>

#include "../entities/rendered_entity.h"
#include "../entities/camera.h"

namespace gigno {

	giSwapChain::giSwapChain(const giDevice &device, const giWindow *window, const std::string &vertShaderPath, const std::string &fragShaderPath)
	{
		CreateVkSwapChain(device.GetDevice(), device.GetPhysicalSwapChainSupport(), device.GetPhysicalDeviceQueueFamilyIndices(), device.GetSurface(), window, true);
		CreateImageViews(device.GetDevice());
		CreateRenderPass(device.GetDevice(), device.GetPhysicalDevice());
		CreateCommandPool(device.GetDevice(), device.GetPhysicalDeviceQueueFamilyIndices());
		CreateDepthResources(device.GetDevice(), device.GetPhysicalDevice());
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

	void giSwapChain::CreatePipelineLayout(VkDevice device) {
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(PushConstantData_t);

		VkPipelineLayoutCreateInfo createInfo;
		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.setLayoutCount = 0;
		createInfo.pSetLayouts = nullptr;
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

	void giSwapChain::RecordCommandBuffer(uint32_t index, uint32_t imageIndex, const std::vector<const RenderedEntity *> &renderedEntities, const Camera *camera) {
		ASSERT(index < MAX_FRAMES_IN_FLIGHT);
		RecordCommandBuffer(m_CommandBuffers[index], imageIndex, renderedEntities, camera);
	}

	void giSwapChain::RecordCommandBuffer(VkCommandBuffer buffer, uint32_t imageIndex, const std::vector<const RenderedEntity *> &renderedEntities, const Camera *camera) {
		VkCommandBufferBeginInfo bufferBeginInfo{};
		bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		bufferBeginInfo.flags = 0;
		bufferBeginInfo.pInheritanceInfo = nullptr;

		VkResult result = vkBeginCommandBuffer(buffer, &bufferBeginInfo);
		if (result != VK_SUCCESS) {
			ERR_MSG("Failed to Begin Command Buffer ! Vulkan Error Code : " << (int)result);
		}

		std::array<VkClearValue, 2> clearValues = {};
		clearValues[0].color = { {0.1f, 0.1f, 0.1f, 1.0f} };
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

		if (camera) {
			RenderEntities(buffer, renderedEntities, camera);
		}

		vkCmdEndRenderPass(buffer);

		result = vkEndCommandBuffer(buffer);
		if (result != VK_SUCCESS) {
			ERR_MSG("Failed to end Command Buffer ! Vulkan Error Code : " << (int)result);
		}
	}

	void giSwapChain::RenderEntities(VkCommandBuffer buffer, const std::vector<const RenderedEntity *> &entities, const Camera *camera) {
		vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline.get()->GetVkPipeline());

		glm::mat4 proView = camera->GetProjection() * camera->GetViewMatrix();

		for (const RenderedEntity *entity : entities) {
			PushConstantData_t push{};
			push.transform = proView * entity->Transform.TransformationMatrix();
			push.color = entity->Color;

			vkCmdPushConstants(buffer, m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantData_t), &push);
			entity->pModel->Bind(buffer);
			entity->pModel->Draw(buffer);
		}
	}

}
