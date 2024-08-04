#pragma once

#include "graphics/render_graph_builder.hpp"
#include "platform/vulkan/device.hpp"
#include "platform/vulkan/pipeline.hpp"
#include "platform/vulkan/render_target.hpp"

namespace huedra {

class VulkanRenderPass
{
public:
    VulkanRenderPass() = default;
    ~VulkanRenderPass() = default;

    void init(Device& device, const PipelineBuilder& builder, RenderCommands commands,
              VulkanRenderTarget* renderTarget = nullptr, bool clearRenderTarget = true);
    void cleanup();

    void createFramebuffers();
    void cleanupFramebuffers();

    void begin(VkCommandBuffer commandBuffer);
    void end(VkCommandBuffer commandBuffer);

    VkRenderPass get() { return m_renderPass; }
    RenderCommands getCommands() { return m_commands; };
    Ref<RenderTarget> getRenderTarget() { return p_renderTarget; }
    VulkanPipeline& getPipeline() { return m_pipeline; }
    PipelineType getPipelineType() { return m_pipeline.getBuilder().getType(); }

private:
    void createRenderPass();

    Device* p_device;
    RenderCommands m_commands;
    Ref<RenderTarget> p_renderTarget{nullptr};
    VulkanRenderTarget* p_vkRenderTarget;
    bool m_clearRenderTarget{true};

    VulkanPipeline m_pipeline;
    VkRenderPass m_renderPass;
    std::vector<VkFramebuffer> m_framebuffers;
};

} // namespace huedra
