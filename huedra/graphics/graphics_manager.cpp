#include "graphics_manager.hpp"

#ifdef VULKAN
#include "platform/vulkan/context.hpp"
#endif

namespace huedra {

void GraphicsManager::init()
{
#ifdef VULKAN
    m_context = new VulkanContext();
#endif

    m_context->init();
}

void GraphicsManager::cleanup()
{
    for (auto swapchain : m_swapchains)
    {
        swapchain->cleanup();
        delete swapchain;
    }

    m_context->cleanup();
    delete m_context;
}

void GraphicsManager::createSwapchain(Window* window)
{
    Swapchain* swapchain = m_context->createSwapchain(window);
    m_swapchains.push_back(swapchain);
}

void GraphicsManager::removeSwapchain(size_t index)
{
    Swapchain* swapchain = m_swapchains[index];
    swapchain->cleanup();
    m_context->removeSwapchain(index);
    m_swapchains.erase(m_swapchains.begin() + index);
    delete swapchain;
}

}; // namespace huedra