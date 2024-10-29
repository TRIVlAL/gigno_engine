
#ifndef RENDERING_UTILS_H
#define RENDERING_UTILS_H
#include "vulkan/vulkan.h"

namespace gigno {
    uint32_t FindMemoryTypeIndex(VkPhysicalDevice device, uint32_t typeFilter, VkMemoryPropertyFlags properties);
    
    void CreateBuffer(VkDevice device, VkPhysicalDevice physDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props, VkBuffer &buffer, VkDeviceMemory &bufferMemory);

    void CopyBuffer(VkDevice device, VkBuffer src, VkBuffer dst, VkDeviceSize size, VkCommandPool commandPool, VkQueue queue);
}

#endif
