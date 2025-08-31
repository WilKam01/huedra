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
    ~VulkanRenderTarget() override = default;

    VulkanRenderTarget(const VulkanRenderTarget& rhs) = default;
    VulkanRenderTarget& operator=(const VulkanRenderTarget& rhs) = default;
    VulkanRenderTarget(VulkanRenderTarget&& rhs) = default;
    VulkanRenderTarget& operator=(VulkanRenderTarget&& rhs) = default;

    void init(Device& device, VulkanSwapchain& swapchain, VkFormat format, VkExtent2D extent);
    void init(Device& device, RenderTargetType type, GraphicsDataFormat format, u32 width, u32 height);
    void cleanup();

    Ref<Texture> getColorTexture() override;
    Ref<Texture> getDepthTexture() override;

    VulkanSwapchain* getSwapchain() const { return m_swapchain; }
    VulkanTexture& getVkColorTexture() { return m_texture; }
    VulkanTexture& getVkDepthTexture() { return m_depthTexture; }
    VkFormat getColorFormat() const { return m_texture.getFormat(); }
    VkFormat getDepthFormat() const { return m_depthTexture.getFormat(); }
    u32 getImageCount() const { return m_imageCount; }
    VkExtent2D getExtent() const { return m_extent; }

    void setAvailability(bool available);

private:
    Device* m_device{nullptr};
    VulkanSwapchain* m_swapchain{nullptr};

    VulkanTexture m_texture;
    VulkanTexture m_depthTexture;

    u32 m_imageCount{0};
    VkExtent2D m_extent{.width = 0, .height = 0};
};

} // namespace huedra