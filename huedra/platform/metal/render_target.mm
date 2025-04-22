#include "render_target.hpp"

namespace huedra {

void MetalRenderTarget::init(id<MTLDevice> device, RenderTargetType type, GraphicsDataFormat format, u32 width,
                             u32 height)
{
    RenderTarget::init(type, format, width, height);
    m_device = device;
}

void MetalRenderTarget::cleanup() {}

Ref<Texture> MetalRenderTarget::getColorTexture()
{
    if (usesColor())
    {
        return Ref<Texture>(nullptr);
    }
    return Ref<Texture>(nullptr);
}

Ref<Texture> MetalRenderTarget::getDepthTexture()
{
    if (usesDepth())
    {
        return Ref<Texture>(nullptr);
    }
    return Ref<Texture>(nullptr);
}

} // namespace huedra