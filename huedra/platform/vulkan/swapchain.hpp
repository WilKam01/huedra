#pragma once

#include "platform/vulkan/command_buffer.hpp"
#include "platform/vulkan/device.hpp"
#include "platform/vulkan/render_target.hpp"
#include "window/window.hpp"

namespace huedra {

class VulkanSwapchain
{
public:
    VulkanSwapchain() = default;
    ~VulkanSwapchain() = default;

    void init(Window* window, Device& device, CommandPool& commandPool, VkSurfaceKHR surface, VkRenderPass renderPass);
    void cleanup();

    std::optional<u32> aquireNextImage(u32 frameIndex);
    void handlePresentResult(VkResult result);

    static VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    VkSwapchainKHR get() { return m_swapchain; }
    VkSurfaceKHR getSurface() { return m_surface; }
    VulkanRenderTarget& getRenderTarget() { return m_renderTarget; }
    VkSemaphore getImageAvailableSemaphore(size_t i) { return m_imageAvailableSemaphores[i]; }
    bool canPresent() { return m_canPresent; }

private:
    VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    void recreate();
    void partialCleanup();
    void createSyncObjects();

    void create();

    Window* p_window;
    Device* p_device;

    VkSwapchainKHR m_swapchain;
    VkSurfaceKHR m_surface;
    VkRenderPass m_renderPass;
    VulkanRenderTarget m_renderTarget;

    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    bool m_canPresent{false};
};

} // namespace huedra
