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

void GraphicsManager::update() { m_context->prepareSwapchains(); }

void GraphicsManager::render(RenderGraphBuilder& builder)
{
    if (!builder.empty())
    {
        builder.generateHash();
        m_context->setRenderGraph(builder);

        m_context->render();
        m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }
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

Ref<Texture> GraphicsManager::createTexture(TextureData textureData)
{
    if (textureData.width == 0 || textureData.height == 0)
    {
        log(LogLevel::WARNING, "Could not create texture, width and height can't be 0");
        return Ref<Texture>(nullptr);
    }
    if (textureData.format == GraphicsDataFormat::UNDEFINED)
    {
        log(LogLevel::WARNING, "Could not create texture, format has be defined");
        return Ref<Texture>(nullptr);
    }
    if (textureData.texelSize == 0 || textureData.texels.empty())
    {
        log(LogLevel::WARNING, "Could not create texture, invalid texture data provided");
        return Ref<Texture>(nullptr);
    }
    return Ref<Texture>(m_context->createTexture(textureData));
}

void GraphicsManager::createSwapchain(Window* window, bool renderDepth)
{
    m_context->createSwapchain(window, renderDepth);
}

void GraphicsManager::removeSwapchain(size_t index) { m_context->removeSwapchain(index); }

}; // namespace huedra