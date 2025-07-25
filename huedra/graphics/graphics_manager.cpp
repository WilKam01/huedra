#include "graphics_manager.hpp"
#include "core/log.hpp"

#ifdef VULKAN
#include "platform/vulkan/context.hpp"
#elif defined(METAL)
#include "platform/metal/context.hpp"
#endif

namespace huedra {

void GraphicsManager::init()
{
#ifdef VULKAN
    m_context = new VulkanContext();
#elif defined(METAL)
    m_context = new MetalContext();
#endif

    m_context->init();
    m_slangContext.init();
}

void GraphicsManager::cleanup()
{
    // m_slangContext.cleanup();
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

Ref<Texture> GraphicsManager::createTexture(const TextureData& textureData)
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

Ref<RenderTarget> GraphicsManager::createRenderTarget(RenderTargetType type, GraphicsDataFormat format, u32 width,
                                                      u32 height)
{
    if (width == 0 || height == 0)
    {
        log(LogLevel::WARNING, "Could not create render target, width and height can't be 0");
        return Ref<RenderTarget>(nullptr);
    }
    if (format == GraphicsDataFormat::UNDEFINED)
    {
        log(LogLevel::WARNING, "Could not create render target, format has be defined");
        return Ref<RenderTarget>(nullptr);
    }

    return Ref<RenderTarget>(m_context->createRenderTarget(type, format, width, height));
}

ShaderModule GraphicsManager::createShaderModule(const std::string& name, const u8* sourceCode, u64 sourceCodeLength)
{
    // Need to create string from source for null termination
    std::string sourceString(reinterpret_cast<const char*>(sourceCode),
                             reinterpret_cast<const char*>(sourceCode) + sourceCodeLength);
    return m_slangContext.createModule(name, sourceString);
}

ShaderModule GraphicsManager::createShaderModule(const std::string& name, std::string& sourceString)
{
    return m_slangContext.createModule(name, sourceString);
}

CompiledShaderModule GraphicsManager::compileAndLinkShaderModules(const std::map<ShaderStage, ShaderInput>& inputs)
{
    return m_slangContext.compileAndLinkModules(inputs);
}

void GraphicsManager::removeBuffer(Ref<Buffer> buffer)
{
    if (!buffer.valid())
    {
        log(LogLevel::WARNING, "Could not remove buffer, ref is invalid");
        return;
    }

    m_context->removeBuffer(buffer.get());
}

void GraphicsManager::removeTexture(Ref<Texture> texture)
{
    if (!texture.valid())
    {
        log(LogLevel::WARNING, "Could not remove texture, ref is invalid");
        return;
    }

    m_context->removeTexture(texture.get());
}

void GraphicsManager::removeRenderTarget(Ref<RenderTarget> renderTarget)
{
    if (!renderTarget.valid())
    {
        log(LogLevel::WARNING, "Could not remove render target, ref is invalid");
        return;
    }

    m_context->removeRenderTarget(renderTarget.get());
}

void GraphicsManager::createSwapchain(Window* window, bool renderDepth)
{
    m_context->createSwapchain(window, renderDepth);
}

void GraphicsManager::removeSwapchain(u64 index) { m_context->removeSwapchain(index); }
}; // namespace huedra