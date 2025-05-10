#include "render_target.hpp"
#include "core/global.hpp"
#include "graphics/render_target.hpp"
#include "graphics/texture.hpp"

namespace huedra {

void MetalRenderTarget::init(id<MTLDevice> device, RenderTargetType type, GraphicsDataFormat format, u32 width,
                             u32 height)
{
    RenderTarget::init(type, format, width, height);
    m_device = device;

    if (usesColor())
    {
        m_colorTexture.init(m_device, TextureType::COLOR, format, width, height, GraphicsManager::MAX_FRAMES_IN_FLIGHT);
    }

    if (usesDepth())
    {
        m_depthTexture.init(m_device, TextureType::DEPTH, format, width, height, GraphicsManager::MAX_FRAMES_IN_FLIGHT);
    }
}

void MetalRenderTarget::recreate(u32 width, u32 height)
{
    RenderTarget::init(getType(), getFormat(), width, height);

    if (usesColor())
    {
        m_colorTexture.cleanup();
        m_colorTexture.init(m_device, TextureType::COLOR, getFormat(), width, height,
                            GraphicsManager::MAX_FRAMES_IN_FLIGHT);
    }

    if (usesDepth())
    {
        m_depthTexture.cleanup();
        m_depthTexture.init(m_device, TextureType::DEPTH, getFormat(), width, height,
                            GraphicsManager::MAX_FRAMES_IN_FLIGHT);
    }
}

void MetalRenderTarget::cleanup()
{
    if (usesColor())
    {
        m_colorTexture.cleanup();
    }

    if (usesDepth())
    {
        m_depthTexture.cleanup();
    }
}

Ref<Texture> MetalRenderTarget::getColorTexture()
{
    if (usesColor())
    {
        return Ref<Texture>(&m_colorTexture);
    }
    return Ref<Texture>(nullptr);
}

Ref<Texture> MetalRenderTarget::getDepthTexture()
{
    if (usesDepth())
    {
        return Ref<Texture>(&m_depthTexture);
    }
    return Ref<Texture>(nullptr);
}

void MetalRenderTarget::setAvailability(bool available) { RenderTarget::setAvailability(available); }

} // namespace huedra