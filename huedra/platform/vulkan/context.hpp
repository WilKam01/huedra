#pragma once

#include "graphics/context.hpp"
#include "platform/vulkan/buffer.hpp"
#include "platform/vulkan/device.hpp"
#include "platform/vulkan/instance.hpp"
#include "platform/vulkan/pipeline.hpp"
#include "platform/vulkan/render_context.hpp"
#include "platform/vulkan/render_pass.hpp"
#include "platform/vulkan/resource_set.hpp"
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

    void createSwapchain(Window* window, bool renderDepth) override;
    void removeSwapchain(size_t index) override;

    Buffer* createBuffer(BufferType type, BufferUsageFlags usage, u64 size, void* data) override;
    ResourceSet* createResourceSet(const std::string& renderPass, u32 setIndex) override;

    void setRenderGraph(RenderGraphBuilder& builder) override;
    void render() override;

private:
    VkSurfaceKHR createSurface(Window* window);
    VkRenderPass createRenderPass(VkFormat format, VkFormat depthFormat);

    void submitGraphicsQueue();
    void presentSwapchains();

    Instance m_instance;
    Device m_device;
    CommandPool m_commandPool;
    CommandBuffer m_commandBuffer;
    VulkanBuffer m_stagingBuffer;

    bool m_recordedCommands{false};

    std::vector<VkSurfaceKHR> m_surfaces;
    std::vector<VulkanSwapchain*> m_swapchains;
    std::vector<VulkanBuffer*> m_buffers;
    std::vector<VulkanResourceSet*> m_resourceSets;

    std::map<std::string, VulkanRenderPass*> m_renderPasses;

    VkViewport m_viewport;
    VkRect2D m_scissor;

    std::vector<VkFence> m_renderingInFlightFences;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
};

} // namespace huedra