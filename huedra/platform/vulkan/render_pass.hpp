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

    void init(Device& device, VulkanPipeline* pipeline, RenderCommands commands, VkRenderPass renderPass,
              VulkanRenderTarget* renderTarget = nullptr, bool clearRenderTarget = true);
    void cleanup();

    void createFramebuffers();
    void cleanupFramebuffers();

    void begin(VkCommandBuffer commandBuffer);
    void end(VkCommandBuffer commandBuffer);

    VkRenderPass get() { return m_renderPass; }
    RenderCommands getCommands() { return m_commands; };
    Ref<Pipeline> getPipeline() { return p_pipeline; }
    Ref<RenderTarget> getRenderTarget() { return p_renderTarget; }

private:
    void createRenderPass();

    Device* p_device;
    Ref<Pipeline> p_pipeline{nullptr};
    Ref<RenderTarget> p_renderTarget{nullptr};
    RenderCommands m_commands;
    bool m_clearRenderTarget{true};

    VulkanPipeline* p_vkPipeline;
    VulkanRenderTarget* p_vkRenderTarget;

    VkRenderPass m_renderPass;
    std::vector<VkFramebuffer> m_framebuffers;
};

} // namespace huedra
