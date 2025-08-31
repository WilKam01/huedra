#include "swapchain.hpp"
#include "core/log.hpp"
#include "graphics/graphics_manager.hpp"

namespace huedra {

void VulkanSwapchain::init(Window* window, Device& device, VkSurfaceKHR surface, bool renderDepth)
{
    m_window = window;
    m_device = &device;
    m_surface = surface;
    m_renderDepth = renderDepth;

    create();
    m_window->setRenderTarget(Ref<RenderTarget>(&m_renderTarget));
}

void VulkanSwapchain::cleanup()
{
    m_device->waitIdle();

    partialCleanup();
    m_renderTarget.cleanup();
}

void VulkanSwapchain::aquireNextImage()
{
    if (m_window->isMinimized())
    {
        m_renderTarget.setAvailability(false);
        return;
    }

    if (m_alreadyAquiredFrame)
    {
        m_renderTarget.setAvailability(true);
        return;
    }

    m_renderTarget.setAvailability(false);

    VkResult result =
        vkAcquireNextImageKHR(m_device->getLogical(), m_swapchain, UINT64_MAX,
                              m_imageAvailableSemaphores[m_semaphoreIndex], VK_NULL_HANDLE, &m_imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreate();
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        log(LogLevel::ERR, "Failed to acquire swap chain image!");
    }
    else
    {
        m_alreadyWaitedOnFrame = false;
        m_alreadyAquiredFrame = true;
        m_renderTarget.setAvailability(true);
    }
}

void VulkanSwapchain::handlePresentResult(VkResult result)
{
    m_alreadyAquiredFrame = false;
    m_semaphoreIndex = (m_semaphoreIndex + 1) % m_imageAvailableSemaphores.size();
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        recreate();
    }
}

VkPresentModeKHR VulkanSwapchain::choosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanSwapchain::chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<u32>::max())
    {
        return capabilities.currentExtent;
    }
    WindowRect rect = m_window->getRect();
    VkExtent2D actualExtent = {rect.screenWidth, rect.screenHeight};

    actualExtent.width =
        std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height =
        std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return actualExtent;
}

void VulkanSwapchain::recreate()
{
    // TODO: Wait for window events?

    m_device->waitIdle();
    partialCleanup();

    create();
}

void VulkanSwapchain::partialCleanup()
{
    m_renderTarget.cleanup();
    vkDestroySwapchainKHR(m_device->getLogical(), m_swapchain, nullptr);

    for (auto& semaphore : m_imageAvailableSemaphores)
    {
        vkDestroySemaphore(m_device->getLogical(), semaphore, nullptr);
    }
}

void VulkanSwapchain::create()
{
    m_imageAvailableSemaphores.resize(GraphicsManager::MAX_FRAMES_IN_FLIGHT + 1);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for (auto& semaphore : m_imageAvailableSemaphores)
    {
        if (vkCreateSemaphore(m_device->getLogical(), &semaphoreInfo, nullptr, &semaphore) != VK_SUCCESS)
        {
            log(LogLevel::ERR, "Failed to create swap chain synchronization objects!");
        }
    }

    VulkanSurfaceSupport surfaceSupport = Device::querySurfaceSupport(m_device->getPhysical(), m_surface);
    VkSurfaceFormatKHR surfaceFormat = Device::chooseSurfaceFormat(surfaceSupport.formats);
    VkPresentModeKHR presentMode = choosePresentMode(surfaceSupport.presentModes);
    VkExtent2D extent = chooseExtent(surfaceSupport.capabilities);

    u32 imageCount = surfaceSupport.capabilities.minImageCount + 1;
    if (surfaceSupport.capabilities.maxImageCount > 0 && imageCount > surfaceSupport.capabilities.maxImageCount)
    {
        imageCount = surfaceSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage =
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT;

    u32 graphicsFamily = m_device->getGraphicsQueueFamilyIndex();
    u32 presentFamily = m_device->getPresentQueueFamilyIndex();
    std::array<u32, 2> queueFamilyIndices = {graphicsFamily, presentFamily};

    if (graphicsFamily != presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = surfaceSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(m_device->getLogical(), &createInfo, nullptr, &m_swapchain) != VK_SUCCESS)
    {
        log(LogLevel::ERR, "Failed to create swap chain!");
    }

    m_renderTarget.init(*m_device, *this, surfaceFormat.format, extent);
}

} // namespace huedra