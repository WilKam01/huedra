#pragma once

#include "graphics/render_target.hpp"
#include "platform/vulkan/device.hpp"
#include "platform/vulkan/texture.hpp"

namespace huedra {

class VulkanSwapchain;

class VulkanRenderTarget : public RenderTarget
{
public:
    VulkanRenderTarget() = default;
    ~VulkanRenderTarget() = default;

    void init(Device& device, CommandPool& commandPool, VulkanSwapchain& swapchain, VkFormat format, VkExtent2D extent,
              VkRenderPass renderPass);
    void init(Device& device, CommandPool& commandPool, RenderTargetType type, GraphicsDataFormat format, u32 width,
              u32 height, VkRenderPass renderPass);
    void cleanup() override;

    void prepareNextFrame(u32 frameIndex) override;

    VkFramebuffer getFramebuffer() { return m_framebuffers[m_currentImageIndex]; }
    u32 getImageIndex() { return m_currentImageIndex; }
    VkExtent2D getExtent() { return m_extent; }

private:
    void createFramebuffers(VkRenderPass renderPass);

    Device* p_device{nullptr};
    VulkanSwapchain* p_swapchain{nullptr};

    VulkanTexture m_texture;
    VulkanTexture m_depthTexture;
    std::vector<VkFramebuffer> m_framebuffers;

    u32 m_currentImageIndex{0};
    VkExtent2D m_extent;
};

} // namespace huedra