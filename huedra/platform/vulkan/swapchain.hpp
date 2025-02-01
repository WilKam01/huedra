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

    void init(Window* window, Device& device, VkSurfaceKHR surface, bool renderDepth);
    void cleanup();

    void aquireNextImage();
    void handlePresentResult(VkResult result);

    VkSwapchainKHR get() { return m_swapchain; }
    VkSurfaceKHR getSurface() { return m_surface; }
    VulkanRenderTarget& getRenderTarget() { return m_renderTarget; }
    VkSemaphore getImageAvailableSemaphore() { return m_imageAvailableSemaphores[m_semaphoreIndex]; }
    bool renderDepth() { return m_renderDepth; }
    bool alreadyWaited() { return m_alreadyWaitedOnFrame; }
    bool canPresent() { return m_renderTarget.isAvailable(); }
    u32 getImageIndex() { return m_imageIndex; }

    void setAlreadyWaited() { m_alreadyWaitedOnFrame = true; }

private:
    VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    void recreate();
    void partialCleanup();

    void create();

    Window* p_window;
    Device* p_device;

    VkSwapchainKHR m_swapchain;
    VkSurfaceKHR m_surface;
    VulkanRenderTarget m_renderTarget;
    bool m_renderDepth;

    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    bool m_alreadyAquiredFrame{false};
    bool m_alreadyWaitedOnFrame{false};
    u32 m_imageIndex{0};
    u32 m_semaphoreIndex{0};
};

} // namespace huedra
