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
    std::vector<TargetInfo> getTargetInfo() { return p_targetInfos; }

    void setInitialColorLayout(u32 index, VkImageLayout layout) { p_targetInfos[index].initialColorLayout = layout; }
    void setFinalColorLayout(u32 index, VkImageLayout layout) { p_targetInfos[index].finalColorLayout = layout; }
    void setInitialDepthLayout(u32 index, VkImageLayout layout) { p_targetInfos[index].initialDepthLayout = layout; }
    void setFinalDepthLayout(u32 index, VkImageLayout layout) { p_targetInfos[index].finalDepthLayout = layout; }

private:
    void createRenderPass();

    Device* p_device;
    RenderPassBuilder m_builder;

    std::vector<TargetInfo> p_targetInfos;
    VulkanRenderTarget* p_swapchainTarget{nullptr};

    VulkanPipeline m_pipeline;
    VkRenderPass m_renderPass;
    std::vector<VkFramebuffer> m_framebuffers;
};

} // namespace huedra
