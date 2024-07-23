#pragma once

#include "graphics/render_target.hpp"
#include "platform/vulkan/device.hpp"

namespace huedra {

class VulkanSwapchain;

class VulkanRenderTarget : public RenderTarget
{
public:
    VulkanRenderTarget() = default;
    ~VulkanRenderTarget() = default;

    void init(Device& device, VkFormat format, VkExtent2D extent, VkRenderPass renderPass,
              VulkanSwapchain* swapchain = nullptr);
    void cleanup() override;

    void prepareNextFrame(u32 frameIndex) override;

    VkFramebuffer getFramebuffer() { return m_framebuffers[m_currentImageIndex]; }
    u32 getImageIndex() { return m_currentImageIndex; }
    VkExtent2D getExtent() { return m_extent; }

private:
    void createImageViews();
    void createFramebuffers(VkRenderPass renderPass);

    Device* p_device{nullptr};
    VulkanSwapchain* p_swapchain{nullptr};

    std::vector<VkImage> m_images;
    std::vector<VkDeviceMemory> m_imageMemories; // Only when swapchain not used

    std::vector<VkImageView> m_imageViews;
    std::vector<VkFramebuffer> m_framebuffers;

    u32 m_currentImageIndex{0};
    VkFormat m_format;
    VkExtent2D m_extent;
};

} // namespace huedra