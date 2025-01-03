#ifndef MODEL_H
#define MODEL_H

#include "vulkan/vulkan.h"
#include "glm/glm.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

#include <vector>
#include <array>

namespace gigno {
	typedef uint32_t indice_t;
	class Device;

	struct Vertex {
		Vertex(){};
		Vertex(glm::vec3 pos, glm::vec3 col) :
			Position{ pos }, Color{ col } {}
		glm::vec3 Position{};
		glm::vec3 Color{};
		glm::vec3 Normal{glm::sqrt(3.0f)};
		glm::vec2 uv{};

		static VkVertexInputBindingDescription GetBindingDescription();
		static std::array<VkVertexInputAttributeDescription, 4> GetAttributeDescriptions();

		// Need Equal operator for unordered map
		bool operator==(const Vertex &other) const {
			return other.Position == Position && other.Color == Color && other.Normal == Normal && other.uv == uv;
		}
	};
}

namespace std {

	//Need Vertex to be hashable for unordered map
	template<>
	struct hash<gigno::Vertex> {
		size_t operator()(const gigno::Vertex &key) const {
			return hash<glm::vec3>()(key.Position) ^ hash<glm::vec3>()(key.Color) ^ 
			hash<glm::vec3>()(key.Normal) ^ hash<glm::vec2>()(key.uv);
		}
	};

}

namespace gigno{

	struct ModelData_t {
		std::vector<Vertex> Vertices{};
		std::vector<indice_t> Indices{};

		static ModelData_t FromObjFile(const char *path);
		static ModelData_t ErrorModel(); //Defined in error_model.cpp
	};

	class giModel {
	public:
		static VkIndexType GetIndexType() {return VK_INDEX_TYPE_UINT32;} 

		giModel();
		giModel(const Device &device, const ModelData_t &data, VkCommandPool commandPool);
		~giModel();

		void CleanUp(VkDevice device);

		void Bind(VkCommandBuffer buffer);
		void Draw(VkCommandBuffer buffer);

	private:
		void CreateVertexBuffer(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool commandPool, VkQueue queue);
		void CreateIndexBuffer(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool commandPool, VkQueue queue);


		std::vector<Vertex> m_Vertices;
		std::vector<indice_t> m_Indices;

		VkBuffer m_VertexBuffer;
		VkDeviceMemory m_VertexBufferMemory;
		VkBuffer m_IndexBuffer;
		VkDeviceMemory m_IndexBufferMemory;
	};

}

#endif
