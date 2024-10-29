#include "model.h"
#include "device.h"
#include "../error_macros.h"
#include "rendering_utils.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "../vendor/tiny_object_loader/tiny_obj_loader.h"

#include "../application.h"

namespace gigno
{

	VkVertexInputBindingDescription Vertex::GetBindingDescription() {
		VkVertexInputBindingDescription description{};
		description.binding = 0;
		description.stride = sizeof(Vertex);
		description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return description;
	}

	std::array<VkVertexInputAttributeDescription, 4> Vertex::GetAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 4> descriptions{};

		descriptions[0] = {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, Position)};
		descriptions[1] = {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, Color)};
		descriptions[2] = {2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, Normal)};
		descriptions[3] = {3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)};

		return descriptions;
	}

	ModelData_t ModelData_t::FromObjFile(const char *path) {
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		
		std::string warn;
		std::string err;

		if(!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path)) {
			ERR_MSG_V(ModelData_t{}, "Tiny Object Loader Error ");
		}

		ModelData_t data{};
		std::unordered_map<Vertex, uint32_t> uniqueVertices{};
		for(const tinyobj::shape_t &shape : shapes) {
			data.Indices.reserve(shape.mesh.indices.size());
			for(const tinyobj::index_t& index : shape.mesh.indices) {
				Vertex vertex{};

				vertex.Position = { attrib.vertices[3 * index.vertex_index + 0],
									attrib.vertices[3 * index.vertex_index + 1],
									attrib.vertices[3 * index.vertex_index + 2]};
				
				uint32_t colorIndex = 3 * index.vertex_index + 2;
				if(colorIndex < attrib.colors.size()) {
					vertex.Color = {attrib.colors[colorIndex - 0],
									attrib.colors[colorIndex - 1],
									attrib.colors[colorIndex - 2]};
				}

				vertex.Normal = {attrib.normals[3 * index.normal_index + 0],
								attrib.normals[3 * index.normal_index + 1],
								attrib.normals[3 * index.normal_index + 2]};
				vertex.uv = {attrib.texcoords[2 * index.texcoord_index + 0],
							 attrib.texcoords[2 * index.texcoord_index + 1]};

				if(uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = static_cast<uint32_t>(data.Vertices.size());
					data.Vertices.push_back(vertex);
				}

				data.Indices.push_back(uniqueVertices[vertex]);
			}
		}

		return data;
	}

	giModel::giModel(const Device &device, const ModelData_t &data, VkCommandPool commandPool) :
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
		VkDeviceSize buffer_size = sizeof(m_Vertices[0]) * m_Vertices.size();

		VkBuffer staging_buffer;
		VkDeviceMemory staging_buffer_memory;

		CreateBuffer(device, physDevice, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					 staging_buffer, staging_buffer_memory);

		void *data;
		vkMapMemory(device, staging_buffer_memory, 0, buffer_size, 0, &data);
		memcpy(data, m_Vertices.data(), (size_t)buffer_size);
		vkUnmapMemory(device, staging_buffer_memory);

		CreateBuffer(device, physDevice, buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
					 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_VertexBuffer, m_VertexBufferMemory);

		CopyBuffer(device, staging_buffer, m_VertexBuffer, buffer_size, commandPool, queue);

		vkDestroyBuffer(device, staging_buffer, nullptr);
		vkFreeMemory(device, staging_buffer_memory, nullptr);
	}

	void giModel::CreateIndexBuffer(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool commandPool, VkQueue queue) {
		VkDeviceSize buffer_size = sizeof(indice_t) * m_Indices.size();

		VkBuffer staging_buffer;
		VkDeviceMemory staging_buffer_memory;

		CreateBuffer(device, physDevice, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					 staging_buffer, staging_buffer_memory);

		void *data{};
		vkMapMemory(device, staging_buffer_memory, 0, buffer_size, 0, &data);
		memcpy(data, m_Indices.data(), (size_t)buffer_size);
		vkUnmapMemory(device, staging_buffer_memory);

		CreateBuffer(device, physDevice, buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
					 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_IndexBuffer, m_IndexBufferMemory);

		CopyBuffer(device, staging_buffer, m_IndexBuffer, buffer_size, commandPool, queue);

		vkDestroyBuffer(device, staging_buffer, nullptr);
		vkFreeMemory(device, staging_buffer_memory, nullptr);
	}

}