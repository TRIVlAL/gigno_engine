#include "model.h"
#include "device.h"
#include "../error_macros.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "../vendor/tiny_object_loader/tiny_obj_loader.h"

namespace gigno
{

	VkVertexInputBindingDescription Vertex::GetBindingDescription() {
		VkVertexInputBindingDescription description{};
		description.binding = 0;
		description.stride = sizeof(Vertex);
		description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return description;
	}

	std::array<VkVertexInputAttributeDescription, 2> Vertex::GetAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 2> descriptions{};

		descriptions[0].location = 0;
		descriptions[0].binding = 0;
		descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		descriptions[0].offset = offsetof(Vertex, position);

		descriptions[1].location = 1;
		descriptions[1].binding = 0;
		descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		descriptions[1].offset = offsetof(Vertex, color);

		return descriptions;
	}

	giModel::giModel(const giDevice &device, const std::vector<Vertex> &vertices, const std::vector<indice_t> &indices, VkCommandPool commandPool) :
		m_Vertices{ vertices }, 
		m_Indices{ indices } {
		CreateVertexBuffer(device.GetDevice(), device.GetPhysicalDevice(), commandPool, device.GetGraphicsQueue());
		CreateIndexBuffer(device.GetDevice(), device.GetPhysicalDevice(), commandPool, device.GetGraphicsQueue());
	}

	giModel::~giModel() {

	}

	void giModel::CleanUp(VkDevice device) {
		vkDestroyBuffer(device, m_VertexBuffer, nullptr);
		vkFreeMemory(device, m_VertexBufferMemory, nullptr);

		vkDestroyBuffer(device, m_IndexBuffer, nullptr);
		vkFreeMemory(device, m_IndexBufferMemory, nullptr);
	}

	void giModel::Bind(VkCommandBuffer buffer) {
		VkBuffer buffers[] = { m_VertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(buffer, 0, 1, buffers, offsets);

		vkCmdBindIndexBuffer(buffer, m_IndexBuffer, 0, VK_INDEX_TYPE_UINT16);
	}

	void giModel::Draw(VkCommandBuffer buffer) {
		vkCmdDrawIndexed(buffer, static_cast<uint32_t>(m_Indices.size()), 1, 0, 0, 0);
	}

	void giModel::CreateVertexBuffer(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool commandPool, VkQueue queue) {
		VkDeviceSize bufferSize = sizeof(m_Vertices[0]) * m_Vertices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		CreateBuffer(device, physDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void *data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, m_Vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(device, stagingBufferMemory);

		CreateBuffer(device, physDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_VertexBuffer, m_VertexBufferMemory);

		CopyBuffer(device, stagingBuffer, m_VertexBuffer, bufferSize, commandPool, queue);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	void giModel::CreateIndexBuffer(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool commandPool, VkQueue queue) {
		VkDeviceSize bufferSize = sizeof(m_Indices) * m_Indices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		CreateBuffer(device, physDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void *data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, m_Indices.data(), (size_t)bufferSize);
		vkUnmapMemory(device, stagingBufferMemory);

		CreateBuffer(device, physDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_IndexBuffer, m_IndexBufferMemory);

		CopyBuffer(device, stagingBuffer, m_IndexBuffer, bufferSize, commandPool, queue);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	void giModel::CreateBuffer(VkDevice device, VkPhysicalDevice physDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props, VkBuffer &buffer, VkDeviceMemory &bufferMemory) {
		VkBufferCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.size = size;
		createInfo.usage = usage;
		createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkResult result = vkCreateBuffer(device, &createInfo, nullptr, &buffer);
		if (result != VK_SUCCESS) {
			ERR_MSG("Failed to create Vertex Buffer ! Vulkan Error Code : " << (int)result);
		}

		VkMemoryRequirements memRequirement{};
		vkGetBufferMemoryRequirements(device, buffer, &memRequirement);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.allocationSize = memRequirement.size;
		allocInfo.memoryTypeIndex = FindMemoryTypeIndex(physDevice, memRequirement.memoryTypeBits, props);

		result = vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory);
		if (result != VK_SUCCESS) {
			ERR_MSG("Failed to allocate memory for Vertex Buffer ! Vulkan Error Code : " << (int)result);
		}

		vkBindBufferMemory(device, buffer, bufferMemory, 0);
	}

	void giModel::CopyBuffer(VkDevice device, VkBuffer src, VkBuffer dst, VkDeviceSize size, VkCommandPool commandPool, VkQueue queue) {

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo info{};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &info);

		VkBufferCopy copyRegion{};
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, src, dst, 1, &copyRegion);

		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submit{};
		submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit.commandBufferCount = 1;
		submit.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(queue, 1, &submit, VK_NULL_HANDLE);
		vkQueueWaitIdle(queue);

		vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
	}

	uint32_t giModel::FindMemoryTypeIndex(VkPhysicalDevice device, uint32_t typeFilter, VkMemoryPropertyFlags properties) const {
		VkPhysicalDeviceMemoryProperties memProp{};
		vkGetPhysicalDeviceMemoryProperties(device, &memProp);

		for (uint32_t i = 0; i < memProp.memoryTypeCount; i++) {
			if ((typeFilter & (i << i)) && (memProp.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		ERR_MSG_V("Failed to find suitable memory type !", UINT32_MAX);
	}

}