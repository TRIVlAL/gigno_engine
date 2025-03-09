#include "gui.h"


#include "../error_macros.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#include "../features_usage.h"

#include "../application.h"

#if USE_IMGUI
namespace gigno
{

    void NewFrameImGui()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    bool InitImGui(GLFWwindow *window, const Device &device, VkRenderPass renderPass)
    {
        IMGUI_CHECKVERSION();

        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui_ImplGlfw_InitForVulkan(window, true);

        VkDescriptorPoolSize imgui_pool_sizes[] = {
            {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

        VkDescriptorPoolCreateInfo poolCreateInfo{};
        poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolCreateInfo.maxSets = 1000;
        poolCreateInfo.poolSizeCount = std::size(imgui_pool_sizes);
        poolCreateInfo.pPoolSizes = imgui_pool_sizes;

        VkDescriptorPool imgui_pool{};
        VkResult result = vkCreateDescriptorPool(device.GetDevice(), &poolCreateInfo, nullptr, &imgui_pool);
        if (result != VK_SUCCESS)
        {
            Console::LogError("Failed to create ImGui Descriptor Pool ! Aborting GUI Initialization ! Vulkan Error Code %d ", (int)result);
            return false;
        }

        ImGui_ImplVulkan_InitInfo vulkanInfo{};
        vulkanInfo.Instance = device.GetInstance();
        vulkanInfo.PhysicalDevice = device.GetPhysicalDevice();
        vulkanInfo.Device = device.GetDevice();
        vulkanInfo.Queue = device.GetGraphicsQueue();
        vulkanInfo.DescriptorPool = imgui_pool;
        vulkanInfo.MinImageCount = 3;
        vulkanInfo.RenderPass = renderPass;
        vulkanInfo.ImageCount = 3;
        vulkanInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

        ImGui_ImplVulkan_Init(&vulkanInfo);
        ImGui_ImplVulkan_CreateFontsTexture();

        ImGui::StyleColorsDark();

        NewFrameImGui();

        return true;
    }

    void RenderImGui(VkCommandBuffer commandBuffer)
    {
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    }

    void ShutdownImGui()
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

}
#endif