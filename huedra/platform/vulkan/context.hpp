#pragma once

#include "graphics/context.hpp"
#include "platform/vulkan/device.hpp"
#include "platform/vulkan/instance.hpp"
#include "platform/vulkan/swapchain.hpp"
#include "window/window.hpp"

namespace huedra {

class VulkanContext : public GraphicalContext
{
public:
    VulkanContext() = default;
    ~VulkanContext() = default;

    void init() override;
    void cleanup() override;

    Swapchain* createSwapchain(Window* window) override;
    void removeSwapchain(size_t index) override;
    void recordGraphicsCommands(u32 swapchainIndex, u32 imageIndex) override;

private:
    VkSurfaceKHR createSurface(Window* window);
    void recordCommandBuffer(VkCommandBuffer commandBuffer, VulkanSwapchain* swapchain, u32 imageIndex);
    // TODO: Move byte reading to asset/io manager
    VkShaderModule loadShader(const std::string& path);

    Instance m_instance;
    Device m_device;
    CommandPool m_commandPool;

    std::vector<VulkanSwapchain*> m_swapchains;
    std::vector<VkSurfaceKHR> m_surfaces;

    VkViewport m_viewport;
    VkRect2D m_scissor;

    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_pipeline;
};

} // namespace huedra