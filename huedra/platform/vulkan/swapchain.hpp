#pragma once

#include "graphics/swapchain.hpp"
#include "platform/vulkan/device.hpp"

namespace huedra {

class VulkanSwapchain : public Swapchain
{
public:
    VulkanSwapchain() = default;
    ~VulkanSwapchain() = default;

    void init(Window* window, Device& device, VkSurfaceKHR surface);
    void cleanup() override;

    void recreate() override;

    VkSwapchainKHR get() { return m_swapchain; }
    VkSurfaceKHR getSurface() { return m_surface; }
    VkFormat getFormat() { return m_format; }
    VkExtent2D getExtent() { return m_extent; }
    VkRenderPass getRenderPass() { return m_renderPass; }

    u32 getNumberOfImages() const { return static_cast<u32>(m_images.size()); }
    VkImageView getImageView(size_t i) { return m_imageViews[i]; }
    VkFramebuffer getFramebuffer(size_t i) { return m_framebuffers[i]; }

private:
    VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    void create();
    void createImageViews();
    void createRenderPass();
    void createFramebuffers();

    VkSwapchainKHR m_swapchain;
    VkSurfaceKHR m_surface;
    std::vector<VkImage> m_images;
    std::vector<VkImageView> m_imageViews;
    std::vector<VkFramebuffer> m_framebuffers;

    VkFormat m_format;
    VkExtent2D m_extent;
    VkRenderPass m_renderPass;

    Device* p_device;
};

} // namespace huedra
