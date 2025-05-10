#pragma once

#include "graphics/render_target.hpp"
#include "platform/metal/config.hpp"
#include "platform/metal/texture.hpp"

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
    void recreate(u32 width, u32 height);
    void cleanup();

    Ref<Texture> getColorTexture() override;
    Ref<Texture> getDepthTexture() override;
    MetalTexture& getMetalColorTexture() { return m_colorTexture; }
    MetalTexture& getMetalDepthTexture() { return m_depthTexture; }

    void setAvailability(bool available);

private:
    id<MTLDevice> m_device;
    MetalTexture m_colorTexture;
    MetalTexture m_depthTexture;
};

} // namespace huedra