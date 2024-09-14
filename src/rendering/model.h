#ifndef MODEL_H
#define MODEL_H

#include "vulkan/vulkan.h"
#include "glm/glm.hpp"

#include <vector>
#include <array>

namespace gigno {
	class giDevice;

	struct Vertex {
		Vertex(glm::vec3 pos, glm::vec3 col) :
			position{ pos }, color{ col } {}
		glm::vec3 position;
		glm::vec3 color;

		static VkVertexInputBindingDescription GetBindingDescription();
		static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions();
	};

	class giModel {
	public:
		typedef uint16_t indice_t;

		giModel();
		giModel(const giDevice &device, const std::vector<Vertex> &vertices, const std::vector<indice_t> &indices, VkCommandPool commandPool);
		~giModel();

		void CleanUp(VkDevice device);

		void Bind(VkCommandBuffer buffer);
		void Draw(VkCommandBuffer buffer);

	private:
		void CreateVertexBuffer(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool commandPool, VkQueue queue);
		void CreateIndexBuffer(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool commandPool, VkQueue queue);

		void CreateBuffer(VkDevice device, VkPhysicalDevice physDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props, VkBuffer &buffer, VkDeviceMemory &bufferMemory);
		void CopyBuffer(VkDevice device, VkBuffer src, VkBuffer dst, VkDeviceSize size, VkCommandPool commandPool, VkQueue queue);

		uint32_t FindMemoryTypeIndex(VkPhysicalDevice device, uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

		std::vector<Vertex> m_Vertices;
		std::vector<indice_t> m_Indices;

		VkBuffer m_VertexBuffer;
		VkDeviceMemory m_VertexBufferMemory;
		VkBuffer m_IndexBuffer;
		VkDeviceMemory m_IndexBufferMemory;
	};

}

#endif
