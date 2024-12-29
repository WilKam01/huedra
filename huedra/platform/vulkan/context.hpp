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
    void readBuffer(u64 id, u64 size, void* data) override;
    void writeToBuffer(u64 id, u64 size, void* data) override;

    Texture* createTexture(TextureData textureData) override;

    void setRenderGraph(RenderGraphBuilder& builder) override;
    void render() override;

    VulkanBuffer* getBuffer(u64 id);
    VulkanTexture* getTexture(u64 id);

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

    std::deque<VulkanBuffer> m_buffers;
    std::deque<Buffer> m_bufferHandles;

    std::deque<VulkanTexture> m_textures;
    std::deque<Texture> m_textureHandles;

    RenderGraphBuilder m_curGraph;
    struct PassInfo
    {
        VulkanRenderPass* pass;
        std::vector<DescriptorHandler> descriptorHandlers;
        VkDescriptorPool descriptorPool;
    };
    std::map<std::string, PassInfo> m_renderPasses;

    std::vector<VkFence> m_renderingInFlightFences;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
};

} // namespace huedra