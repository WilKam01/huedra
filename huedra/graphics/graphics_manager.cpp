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
    m_context->cleanup();
    delete m_context;
}

void GraphicsManager::render()
{
    bool shouldRender = !m_renderTargets.empty();
    for (auto& renderTarget : m_renderTargets)
    {
        shouldRender = renderTarget.valid();
        if (shouldRender)
        {
            break;
        }
    }

    if (!shouldRender)
    {
        return;
    }

    m_context->prepareRendering();

    for (auto& renderTarget : m_renderTargets)
    {
        if (!renderTarget.valid())
        {
            continue;
        }

        renderTarget.get()->prepareNextFrame(m_context->getFrameIndex());
        if (renderTarget.get()->isAvailable())
        {
            m_context->recordGraphicsCommands(*renderTarget.get());
        }
    }

    m_context->submitGraphicsQueue();
    m_context->presentSwapchains();
}

void GraphicsManager::addRenderTarget(Ref<RenderTarget> renderTarget) { m_renderTargets.push_back(renderTarget); }

void GraphicsManager::createSwapchain(Window* window) { m_context->createSwapchain(window); }

void GraphicsManager::removeSwapchain(size_t index) { m_context->removeSwapchain(index); }

}; // namespace huedra