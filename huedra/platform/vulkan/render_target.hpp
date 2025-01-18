#pragma once

#include "graphics/render_target.hpp"
#include "platform/vulkan/device.hpp"
#include "platform/vulkan/texture.hpp"

namespace huedra {

class VulkanSwapchain;
class VulkanRenderPass;

class VulkanRenderTarget : public RenderTarget
{
public:
    VulkanRenderTarget() = default;
    ~VulkanRenderTarget() = default;

    void init(Device& device, CommandPool& commandPool, VulkanSwapchain& swapchain, VkFormat format, VkExtent2D extent);
    void init(Device& device, CommandPool& commandPool, RenderTargetType type, GraphicsDataFormat format, u32 width,
              u32 height);
    void cleanup();
    void partialCleanup();

    void addRenderPass(VulkanRenderPass* renderPass);
    void removeRenderPass(VulkanRenderPass* renderPass);

    VulkanSwapchain* getSwapchain() { return p_swapchain; }
    VulkanTexture& getColorTexture() { return m_texture; }
    VulkanTexture& getDepthTexture() { return m_depthTexture; }
    VkFormat getColorFormat() { return m_texture.getFormat(); }
    VkFormat getDepthFormat() { return m_depthTexture.getFormat(); }
    u32 getImageCount() { return m_imageCount; }
    VkExtent2D getExtent() { return m_extent; }

    void setAvailability(bool available);

private:
    Device* p_device{nullptr};
    VulkanSwapchain* p_swapchain{nullptr};
    std::vector<VulkanRenderPass*> p_renderPasses;

    VulkanTexture m_texture;
    VulkanTexture m_depthTexture;

    u32 m_imageCount{0};
    VkExtent2D m_extent;
};

} // namespace huedra