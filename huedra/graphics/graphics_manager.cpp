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
    m_context->render();
    m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
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

Ref<ResourceSet> GraphicsManager::createResourceSet(const std::string renderPass, u32 setIndex)
{
    return Ref<ResourceSet>(m_context->createResourceSet(renderPass, setIndex));
}

void GraphicsManager::setRenderGraph(RenderGraphBuilder& builder) { m_context->setRenderGraph(builder); }

void GraphicsManager::createSwapchain(Window* window, bool renderDepth)
{
    m_context->createSwapchain(window, renderDepth);
}

void GraphicsManager::removeSwapchain(size_t index) { m_context->removeSwapchain(index); }

}; // namespace huedra