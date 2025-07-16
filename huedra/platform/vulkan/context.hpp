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
    ~VulkanContext() override = default;

    VulkanContext(const VulkanContext& rhs) = delete;
    VulkanContext& operator=(const VulkanContext& rhs) = delete;
    VulkanContext(VulkanContext&& rhs) = delete;
    VulkanContext& operator=(VulkanContext&& rhs) = delete;

    void init() override;
    void cleanup() override;

    void createSwapchain(Window* window, bool renderDepth) override;
    void removeSwapchain(u64 index) override;

    Buffer* createBuffer(BufferType type, BufferUsageFlags usage, u64 size, void* data) override;
    Texture* createTexture(const TextureData& textureData) override;
    RenderTarget* createRenderTarget(RenderTargetType type, GraphicsDataFormat format, u32 width, u32 height) override;

    void removeBuffer(Buffer* buffer) override;
    void removeTexture(Texture* texture) override;
    void removeRenderTarget(RenderTarget* renderTarget) override;

    void prepareSwapchains() override;
    void setRenderGraph(RenderGraphBuilder& builder) override;
    void render() override;

    VkSampler getSampler(const SamplerSettings& settings);

private:
    struct ResourceTransition
    {
        VulkanTexture* texture{nullptr};
        VkImageLayout newLayout{VK_IMAGE_LAYOUT_UNDEFINED};
        VkPipelineStageFlags newStage{VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT};
    };
    struct PassInfo
    {
        VulkanRenderPass* pass{nullptr};
        std::vector<ResourceTransition> transitions;
        std::vector<DescriptorHandler> descriptorHandlers;
        VkDescriptorPool descriptorPool;
    };

    void createDescriptorHandlers(const RenderPassBuilder& builder, PassInfo& passInfo);
    void createSampler(const SamplerSettings& settings);

    void submitGraphicsQueue(u32 batchIndex);
    void submitComputeQueue(u32 batchIndex);
    void presentSwapchains();

    static void transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format,
                                      VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccessMask,
                                      VkAccessFlags dstAccessMask, VkPipelineStageFlags srcStageMask,
                                      VkPipelineStageFlags dstStageMask);

    Instance m_instance;
    Device m_device;
    VulkanBuffer m_stagingBuffer;

    std::vector<VkSurfaceKHR> m_surfaces;
    std::vector<VulkanSwapchain*> m_swapchains;
    std::set<VulkanSwapchain*> m_activeSwapchains;

    std::deque<VulkanBuffer> m_buffers;
    std::deque<VulkanTexture> m_textures;
    std::deque<VulkanRenderTarget> m_renderTargets;

    RenderGraphBuilder m_curGraph;
    struct PassBatch
    {
        std::vector<PassInfo> passes;
        std::set<VulkanSwapchain*> swapchains;
        bool useGraphicsQueue{false};
        bool useComputeQueue{false};
    };
    std::vector<PassBatch> m_passBatches;
    bool m_usingGraphicsQueue{false};
    bool m_usingComputeQueue{false};
    u32 m_lastGraphicsBatch{0};
    u32 m_lastComputeBatch{0};

    struct SamplerInfo
    {
        SamplerSettings settings;
        VkSampler sampler{nullptr};
    };
    std::vector<SamplerInfo> m_samplers;

    CommandPool m_graphicsCommandPool;
    CommandBuffer m_graphicsCommandBuffer;
    std::array<std::vector<VkSemaphore>, 2> m_graphicsSyncSemaphores;
    u32 m_curGraphicsSemphoreIndex{0};
    std::vector<VkFence> m_graphicsFrameInFlightFences;

    CommandPool m_computeCommandPool;
    CommandBuffer m_computeCommandBuffer;
    std::array<std::vector<VkSemaphore>, 2> m_computeSyncSemaphores;
    u32 m_curComputeSemphoreIndex{0};
    std::vector<VkFence> m_computeFrameInFlightFences;
};

} // namespace huedra