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
    bool shouldRender = !m_renderPasses.empty();
    for (auto& renderPass : m_renderPasses)
    {
        shouldRender = renderPass.getRenderTarget().valid();
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

    for (auto& renderPass : m_renderPasses)
    {
        if (!renderPass.getRenderTarget().valid())
        {
            continue;
        }

        RenderTarget* renderTarget = renderPass.getRenderTarget().get();
        renderTarget->prepareNextFrame(m_context->getFrameIndex());
        if (renderTarget->isAvailable())
        {
            m_context->recordGraphicsCommands(renderPass);
        }
    }

    m_context->submitGraphicsQueue();
    m_context->presentSwapchains();
}

void GraphicsManager::addRenderPass(RenderPass renderPass) { m_renderPasses.push_back(renderPass); }

void GraphicsManager::createSwapchain(Window* window) { m_context->createSwapchain(window); }

void GraphicsManager::removeSwapchain(size_t index) { m_context->removeSwapchain(index); }

}; // namespace huedra