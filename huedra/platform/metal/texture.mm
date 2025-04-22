#include "texture.hpp"
#include "graphics/texture.hpp"
#include "resources/texture/data.hpp"

namespace huedra {

void MetalTexture::init(id<MTLDevice> device, const TextureData& data)
{
    Texture::init(data.width, data.height, data.format, TextureType::COLOR);
    m_device = device;
}

void MetalTexture::cleanup() {}

} // namespace huedra