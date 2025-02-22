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

    VulkanSwapchain(const VulkanSwapchain& rhs) = default;
    VulkanSwapchain& operator=(const VulkanSwapchain& rhs) = default;
    VulkanSwapchain(VulkanSwapchain&& rhs) = default;
    VulkanSwapchain& operator=(VulkanSwapchain&& rhs) = default;

    void init(Window* window, Device& device, VkSurfaceKHR surface, bool renderDepth);
    void cleanup();

    void aquireNextImage();
    void handlePresentResult(VkResult result);

    VkSwapchainKHR get() const { return m_swapchain; }
    VkSurfaceKHR getSurface() const { return m_surface; }
    VulkanRenderTarget& getRenderTarget() { return m_renderTarget; }
    VkSemaphore getImageAvailableSemaphore() const { return m_imageAvailableSemaphores[m_semaphoreIndex]; }
    bool renderDepth() const { return m_renderDepth; }
    bool alreadyWaited() const { return m_alreadyWaitedOnFrame; }
    bool canPresent() const { return m_renderTarget.isAvailable(); }
    u32 getImageIndex() const { return m_imageIndex; }

    void setAlreadyWaited() { m_alreadyWaitedOnFrame = true; }

private:
    static VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    void recreate();
    void partialCleanup();

    void create();

    Window* m_window{nullptr};
    Device* m_device{nullptr};

    VkSwapchainKHR m_swapchain{nullptr};
    VkSurfaceKHR m_surface{nullptr};
    VulkanRenderTarget m_renderTarget;
    bool m_renderDepth{true};

    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    bool m_alreadyAquiredFrame{false};
    bool m_alreadyWaitedOnFrame{false};
    u32 m_imageIndex{0};
    u32 m_semaphoreIndex{0};
};

} // namespace huedra
