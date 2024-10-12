#include "rendering_utils.h"
#include "../application.h"
#include "../error_macros.h"

namespace gigno
{
    uint32_t FindMemoryTypeIndex(VkPhysicalDevice device, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProp{};
        vkGetPhysicalDeviceMemoryProperties(device, &memProp);

        for (uint32_t i = 0; i < memProp.memoryTypeCount; i++) {
            if ((typeFilter & (i << i)) && (memProp.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        ERR_MSG_V(UINT32_MAX, "Failed to find suitable memory type !");
    }

    void CreateBuffer(VkDevice device, VkPhysicalDevice physDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props, VkBuffer &buffer, VkDeviceMemory &bufferMemory) {
        VkBufferCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.size = size;
        createInfo.usage = usage;
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkResult result = vkCreateBuffer(device, &createInfo, nullptr, &buffer);
        if (result != VK_SUCCESS) {
            ERR_MSG("Failed to create Buffer ! Vulkan Error Code : %d", (int)result);
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
            ERR_MSG("Failed to allocate memory for Buffer ! Vulkan Error Code : %d", (int)result);
        }

        vkBindBufferMemory(device, buffer, bufferMemory, 0);
    }

    void CopyBuffer(VkDevice device, VkBuffer src, VkBuffer dst, VkDeviceSize size, VkCommandPool commandPool, VkQueue queue) {
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
}