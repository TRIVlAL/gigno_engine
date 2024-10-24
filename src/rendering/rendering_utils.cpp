#include "rendering_utils.h"
#include "../application.h"
#include "../error_macros.h"

namespace gigno
{
    uint32_t FindMemoryTypeIndex(VkPhysicalDevice device, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties mem_prop{};
        vkGetPhysicalDeviceMemoryProperties(device, &mem_prop);

        for (uint32_t i = 0; i < mem_prop.memoryTypeCount; i++) {
            if ((typeFilter & (i << i)) && (mem_prop.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        ERR_MSG_V(UINT32_MAX, "Failed to find suitable memory type !");
    }

    void CreateBuffer(VkDevice device, VkPhysicalDevice physDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props, VkBuffer &buffer, VkDeviceMemory &bufferMemory) {
        VkBufferCreateInfo createinfo{};
        createinfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createinfo.pNext = nullptr;
        createinfo.size = size;
        createinfo.usage = usage;
        createinfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkResult result = vkCreateBuffer(device, &createinfo, nullptr, &buffer);
        if (result != VK_SUCCESS) {
            ERR_MSG("Failed to create Buffer ! Vulkan Error Code : %d", (int)result);
        }

        VkMemoryRequirements mem_requirement{};
        vkGetBufferMemoryRequirements(device, buffer, &mem_requirement);

        VkMemoryAllocateInfo allocinfo{};
        allocinfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocinfo.pNext = nullptr;
        allocinfo.allocationSize = mem_requirement.size;
        allocinfo.memoryTypeIndex = FindMemoryTypeIndex(physDevice, mem_requirement.memoryTypeBits, props);

        result = vkAllocateMemory(device, &allocinfo, nullptr, &bufferMemory);
        if (result != VK_SUCCESS) {
            ERR_MSG("Failed to allocate memory for Buffer ! Vulkan Error Code : %d", (int)result);
        }

        vkBindBufferMemory(device, buffer, bufferMemory, 0);
    }

    void CopyBuffer(VkDevice device, VkBuffer src, VkBuffer dst, VkDeviceSize size, VkCommandPool commandPool, VkQueue queue) {
        VkCommandBufferAllocateInfo allocinfo{};
        allocinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocinfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocinfo.commandPool = commandPool;
        allocinfo.commandBufferCount = 1;

        VkCommandBuffer command_buffer;
        vkAllocateCommandBuffers(device, &allocinfo, &command_buffer);

        VkCommandBufferBeginInfo info{};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(command_buffer, &info);

        VkBufferCopy copy_region{};
        copy_region.size = size;
        vkCmdCopyBuffer(command_buffer, src, dst, 1, &copy_region);

        vkEndCommandBuffer(command_buffer);

        VkSubmitInfo submit{};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &command_buffer;

        vkQueueSubmit(queue, 1, &submit, VK_NULL_HANDLE);
        vkQueueWaitIdle(queue);

        vkFreeCommandBuffers(device, commandPool, 1, &command_buffer);
    }
}