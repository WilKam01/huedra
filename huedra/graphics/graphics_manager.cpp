#include "graphics_manager.hpp"
#include "core/log.hpp"

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
        if (!renderPass.getRenderTarget().valid() || !renderPass.getPipeline().valid())
        {
            continue;
        }

        RenderTarget* renderTarget = renderPass.getRenderTarget().get();
        renderTarget->prepareNextFrame(m_currentFrame);
        if (renderTarget->isAvailable())
        {
            m_context->recordGraphicsCommands(renderPass);
        }
    }

    m_context->submitGraphicsQueue();
    m_context->presentSwapchains();

    m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

Ref<Pipeline> GraphicsManager::createPipeline(const PipelineBuilder& pipelineBuilder)
{
    return Ref<Pipeline>(m_context->createPipeline(pipelineBuilder));
}

Ref<Buffer> GraphicsManager::createBuffer(BufferType type, u32 usage, u64 size, void* data)
{
    if (usage == HU_BUFFER_USAGE_UNDEFINED)
    {
        log(LogLevel::WARNING, "Could not create buffer, buffer usage is undefined");
        return Ref<Buffer>(nullptr);
    }

    if (size == 0)
    {
        log(LogLevel::WARNING, "Could not create buffer, size is 0");
        return Ref<Buffer>(nullptr);
    }

    return Ref<Buffer>(m_context->createBuffer(type, static_cast<BufferUsageFlags>(usage), size, data));
}

Ref<ResourceSet> GraphicsManager::createResourceSet(Ref<Pipeline> pipeline, u32 setIndex)
{
    if (!pipeline.valid())
    {
        log(LogLevel::WARNING, "Could not create resource set, pipeline not valid");
        return Ref<ResourceSet>(nullptr);
    }

    return Ref<ResourceSet>(m_context->createResourceSet(pipeline.get(), setIndex));
}

void GraphicsManager::addRenderPass(RenderPass renderPass) { m_renderPasses.push_back(renderPass); }

void GraphicsManager::createSwapchain(Window* window) { m_context->createSwapchain(window); }

void GraphicsManager::removeSwapchain(size_t index) { m_context->removeSwapchain(index); }

}; // namespace huedra