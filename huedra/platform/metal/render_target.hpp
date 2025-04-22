#pragma once

#include "graphics/render_target.hpp"
#include "platform/metal/config.hpp"

namespace huedra {

class MetalRenderTarget : public RenderTarget
{
public:
    MetalRenderTarget() = default;
    ~MetalRenderTarget() = default;

    MetalRenderTarget(const MetalRenderTarget& rhs) = default;
    MetalRenderTarget& operator=(const MetalRenderTarget& rhs) = default;
    MetalRenderTarget(MetalRenderTarget&& rhs) = default;
    MetalRenderTarget& operator=(MetalRenderTarget&& rhs) = default;

    void init(id<MTLDevice> device, RenderTargetType type, GraphicsDataFormat format, u32 width, u32 height);
    void cleanup();

    Ref<Texture> getColorTexture() override;
    Ref<Texture> getDepthTexture() override;

private:
    id<MTLDevice> m_device;
};

} // namespace huedra