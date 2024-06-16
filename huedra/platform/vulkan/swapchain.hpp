#pragma once

#include "graphics/swapchain.hpp"
#include "platform/vulkan/command_buffer.hpp"
#include "platform/vulkan/device.hpp"

namespace huedra {

class VulkanSwapchain : public Swapchain
{
public:
    VulkanSwapchain() = default;
    ~VulkanSwapchain() = default;

    void init(Window* window, Device& device, CommandPool& commandPool, VkSurfaceKHR surface);
    void cleanup() override;

    bool graphicsReady() override;
    std::optional<u32> acquireNextImage() override;
    void submitGraphicsQueue(u32 imageIndex) override;
    bool present(u32 imageIndex) override;

    static VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    static VkRenderPass createRenderPass(Device& device, VkFormat format);

    VkSwapchainKHR get() { return m_swapchain; }
    VkSurfaceKHR getSurface() { return m_surface; }
    VkFormat getFormat() { return m_format; }
    VkExtent2D getExtent() { return m_extent; }
    VkRenderPass getRenderPass() { return m_renderPass; }

    u32 getCurrentFrame() const { return m_currentFrame; }
    CommandBuffer& getCommandBuffer() { return m_commandBuffer; }
    VkImageView getImageView(size_t index) { return m_imageViews[index]; }
    VkFramebuffer getFramebuffer(size_t index) { return m_framebuffers[index]; }

private:
    VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    void recreate();
    void partialCleanup();
    void createSyncObjects();

    void create();
    void createImageViews();
    void createFramebuffers();

    Device* p_device;

    VkSwapchainKHR m_swapchain;
    CommandBuffer m_commandBuffer;
    VkSurfaceKHR m_surface;
    std::vector<VkImage> m_images;
    std::vector<VkImageView> m_imageViews;
    std::vector<VkFramebuffer> m_framebuffers;

    VkFormat m_format;
    VkExtent2D m_extent;
    VkRenderPass m_renderPass;

    u32 m_currentFrame{0};
    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;
};

} // namespace huedra
