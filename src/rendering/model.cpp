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

	ModelData_t ModelData_t::FromObjFile(const char *path) {
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		
		std::string warn;
		std::string err;

		if(!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path)) {
			ERR_MSG_V("Tiny Object Loader Error :" << warn << " " << err, ModelData_t{});
		}

		ModelData_t data{};
		std::unordered_map<Vertex, uint32_t> uniqueVertices{};
		for(const tinyobj::shape_t &shape : shapes) {
			for(const tinyobj::index_t& index : shape.mesh.indices) {
				Vertex vertex{};

				vertex.position = { attrib.vertices[3 * index.vertex_index + 0],
									attrib.vertices[3 * index.vertex_index + 1],
									attrib.vertices[3 * index.vertex_index + 2]};
				
				uint32_t colorIndex = 3 * index.vertex_index + 2;
				if(colorIndex < attrib.colors.size()) {
					vertex.color = {attrib.colors[colorIndex - 0],
									attrib.colors[colorIndex - 1],
									attrib.colors[colorIndex - 2]};
				}

				vertex.normal = {attrib.normals[3 * index.vertex_index + 0],
								attrib.normals[3 * index.vertex_index + 1],
								attrib.normals[3 * index.vertex_index + 2]};
				vertex.uv = {attrib.texcoords[2 * index.texcoord_index + 0],
							 1.0f - attrib.texcoords[2 * index.texcoord_index + 1]}; //subtract : Invert coordinate to follow texture orientation.

				if(uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = static_cast<uint32_t>(data.Vertices.size());
					data.Vertices.push_back(vertex);
				}

				data.Indices.push_back(uniqueVertices[vertex]);
			}
		}

		return data;
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

	giModel::giModel(const giDevice &device, const ModelData_t &data, VkCommandPool commandPool) :
		m_Vertices{ data.Vertices }, 
		m_Indices{ data.Indices } {
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

		vkCmdBindIndexBuffer(buffer, m_IndexBuffer, 0, GetIndexType());
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
		VkDeviceSize bufferSize = sizeof(indice_t) * m_Indices.size();

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