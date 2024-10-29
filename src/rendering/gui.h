#ifndef GUI_H
#define GUI_H

#include "../features_usage.h"

#if USE_IMGUI

#include "device.h"

#include "imgui.h"

namespace gigno {

    void NewFrameImGui();
    
    bool InitImGui(GLFWwindow *window, const Device &device, const SwapChain &swapchain);


    void RenderImGui(VkCommandBuffer commandBuffer);

    void ShutdownImGui();

}

#endif // USE_IMGUI

#endif