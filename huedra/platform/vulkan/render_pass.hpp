#pragma once

#include "graphics/render_graph_builder.hpp"
#include "platform/vulkan/device.hpp"
#include "platform/vulkan/pipeline.hpp"
#include "platform/vulkan/render_target.hpp"

namespace huedra {

struct TargetInfo
{
    VulkanRenderTarget* renderTarget{nullptr};
    VkImageLayout initialColorLayout{VK_IMAGE_LAYOUT_UNDEFINED};
    VkImageLayout finalColorLayout{VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkImageLayout initialDepthLayout{VK_IMAGE_LAYOUT_UNDEFINED};
    VkImageLayout finalDepthLayout{VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
};

class VulkanRenderPass
{
public:
    VulkanRenderPass() = default;
    ~VulkanRenderPass() = default;

    VulkanRenderPass(const VulkanRenderPass& rhs) = default;
    VulkanRenderPass& operator=(const VulkanRenderPass& rhs) = default;
    VulkanRenderPass(VulkanRenderPass&& rhs) = default;
    VulkanRenderPass& operator=(VulkanRenderPass&& rhs) = default;

    void init(Device& device, const RenderPassBuilder& builder);
    void create();
    void cleanup();

    void createFramebuffers();
    void cleanupFramebuffers();

    void begin(VkCommandBuffer commandBuffer);
    void end(VkCommandBuffer commandBuffer);

    VkRenderPass get() { return m_renderPass; }
    RenderPassBuilder& getBuilder() { return m_builder; }
    RenderCommands getCommands() { return m_builder.getCommands(); };
    VulkanPipeline& getPipeline() { return m_pipeline; }
    PipelineType getPipelineType() { return m_pipeline.getBuilder().getType(); }
    std::vector<TargetInfo> getTargetInfo() { return m_targetInfos; }

    void setInitialColorLayout(u32 index, VkImageLayout layout) { m_targetInfos[index].initialColorLayout = layout; }
    void setFinalColorLayout(u32 index, VkImageLayout layout) { m_targetInfos[index].finalColorLayout = layout; }
    void setInitialDepthLayout(u32 index, VkImageLayout layout) { m_targetInfos[index].initialDepthLayout = layout; }
    void setFinalDepthLayout(u32 index, VkImageLayout layout) { m_targetInfos[index].finalDepthLayout = layout; }

private:
    void createRenderPass();

    Device* m_device{nullptr};
    RenderPassBuilder m_builder;

    std::vector<TargetInfo> m_targetInfos;
    VulkanSwapchain* m_swapchain{nullptr};

    VulkanPipeline m_pipeline;
    VkRenderPass m_renderPass{nullptr};
    std::vector<VkFramebuffer> m_framebuffers;
};

} // namespace huedra
