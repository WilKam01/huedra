#pragma once

#include "graphics/context.hpp"
#include "platform/vulkan/buffer.hpp"
#include "platform/vulkan/descriptor_handler.hpp"
#include "platform/vulkan/device.hpp"
#include "platform/vulkan/instance.hpp"
#include "platform/vulkan/pipeline.hpp"
#include "platform/vulkan/render_context.hpp"
#include "platform/vulkan/render_pass.hpp"
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
    Texture* createTexture(TextureData textureData) override;

    void prepareSwapchains() override;
    void setRenderGraph(RenderGraphBuilder& builder) override;
    void render() override;

    VulkanBuffer* getBuffer(u64 id);
    VulkanTexture* getTexture(u64 id);

private:
    VkSurfaceKHR createSurface(Window* window);

    void submitGraphicsQueue(u32 batchIndex);
    void presentSwapchains();

    Instance m_instance;
    Device m_device;
    CommandPool m_commandPool;
    CommandBuffer m_commandBuffer;
    VulkanBuffer m_stagingBuffer;

    std::vector<VkSurfaceKHR> m_surfaces;
    std::vector<VulkanSwapchain*> m_swapchains;
    std::set<VulkanSwapchain*> m_activeSwapchains;

    std::deque<VulkanBuffer> m_buffers;
    std::deque<VulkanTexture> m_textures;

    RenderGraphBuilder m_curGraph;
    struct PassInfo
    {
        VulkanRenderPass* pass;
        std::vector<DescriptorHandler> descriptorHandlers;
        VkDescriptorPool descriptorPool;
    };
    struct PassBatch
    {
        std::vector<PassInfo> passes;
        std::set<VulkanSwapchain*> swapchains;
    };
    std::vector<PassBatch> m_passBatches;

    std::vector<VkFence> m_frameInFlightFences;
    std::array<std::vector<VkSemaphore>, 2> m_graphicsSyncSemaphores;
    u32 m_curGraphicsSemphoreIndex{0};
};

} // namespace huedra