#pragma once

#include "graphics/texture.hpp"
#include "platform/metal/config.hpp"
#include "resources/texture/data.hpp"

namespace huedra {

class MetalTexture : public Texture
{
public:
    MetalTexture() = default;
    ~MetalTexture() = default;

    MetalTexture(const MetalTexture& rhs) = default;
    MetalTexture& operator=(const MetalTexture& rhs) = default;
    MetalTexture(MetalTexture&& rhs) = default;
    MetalTexture& operator=(MetalTexture&& rhs) = default;

    void init(id<MTLDevice> device, id<MTLCommandQueue> commandQueue, const TextureData& data); // Static texture
    void init(id<MTLDevice> device, TextureType type, GraphicsDataFormat format, u32 width, u32 height,
              u32 imageCount); // Render target texture
    void cleanup();

    id<MTLTexture> get() const { return m_textures[getIndex()]; }
    id<MTLTexture> getWithIndex(u32 index) const { return m_textures[index]; }

private:
    u32 getIndex() const;

    id<MTLDevice> m_device;
    std::vector<id<MTLTexture>> m_textures;
    bool m_isRenderTargetTexture{false};
};

} // namespace huedra