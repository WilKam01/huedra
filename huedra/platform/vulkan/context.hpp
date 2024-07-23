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

    void createSwapchain(Window* window) override;
    void removeSwapchain(size_t index) override;

    void prepareRendering() override;
    void recordGraphicsCommands(RenderPass& renderPass) override;
    void submitGraphicsQueue() override;
    void presentSwapchains() override;

    u32 getFrameIndex() override { return m_currentFrame; };

private:
    VkSurfaceKHR createSurface(Window* window);
    VkRenderPass createRenderPass(VkFormat format);

    void recordCommandBuffer(VkCommandBuffer commandBuffer, RenderPass& renderPass);
    // TODO: Move byte reading to asset/io manager
    VkShaderModule loadShader(const std::string& path);

    Instance m_instance;
    Device m_device;
    CommandPool m_commandPool;
    CommandBuffer m_commandBuffer;

    u32 m_currentFrame{0};
    bool m_recordedCommands{false};

    std::vector<VulkanSwapchain*> m_swapchains;
    std::vector<VkSurfaceKHR> m_surfaces;

    VkViewport m_viewport;
    VkRect2D m_scissor;

    std::vector<VkFence> m_renderingInFlightFences;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;

    VkRenderPass m_renderPass;
    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_pipeline;
};

} // namespace huedra