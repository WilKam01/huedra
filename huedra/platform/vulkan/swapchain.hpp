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

    void init(Window* window, Device& device, CommandPool& commandPool, VkSurfaceKHR surface, bool renderDepth);
    void cleanup();

    std::optional<u32> aquireNextImage(u32 frameIndex);
    void handlePresentResult(VkResult result);

    VkSwapchainKHR get() { return m_swapchain; }
    VkSurfaceKHR getSurface() { return m_surface; }
    VulkanRenderTarget& getRenderTarget() { return m_renderTarget; }
    VkSemaphore getImageAvailableSemaphore(size_t i) { return m_imageAvailableSemaphores[i]; }
    bool renderDepth() { return m_renderDepth; }
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
    CommandPool* p_commandPool;

    VkSwapchainKHR m_swapchain;
    VkSurfaceKHR m_surface;
    VulkanRenderTarget m_renderTarget;
    bool m_renderDepth;

    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    bool m_canPresent{false};
};

} // namespace huedra
