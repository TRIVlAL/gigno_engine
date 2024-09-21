
#ifndef RENDERING_UTILS_H
#define RENDERING_UTILS_H
#include "vulkan/vulkan.h"
#include "../error_macros.h"

namespace gigno {
    static uint32_t FindMemoryTypeIndex(VkPhysicalDevice device, uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProp{};
        vkGetPhysicalDeviceMemoryProperties(device, &memProp);

        for (uint32_t i = 0; i < memProp.memoryTypeCount; i++)
        {
            if ((typeFilter & (i << i)) && (memProp.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        ERR_MSG_V("Failed to find suitable memory type !", UINT32_MAX);
    }
    
    static void CreateBuffer(VkDevice device, VkPhysicalDevice physDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props, VkBuffer &buffer, VkDeviceMemory &bufferMemory) {
        VkBufferCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.size = size;
        createInfo.usage = usage;
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkResult result = vkCreateBuffer(device, &createInfo, nullptr, &buffer);
        if (result != VK_SUCCESS)
        {
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
        if (result != VK_SUCCESS)
        {
            ERR_MSG("Failed to allocate memory for Vertex Buffer ! Vulkan Error Code : " << (int)result);
        }

        vkBindBufferMemory(device, buffer, bufferMemory, 0);
    }
}

#endif
