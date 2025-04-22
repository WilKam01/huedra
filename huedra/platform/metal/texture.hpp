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

    void init(id<MTLDevice> device, const TextureData& data);
    void cleanup();

private:
    id<MTLDevice> m_device;
};

} // namespace huedra