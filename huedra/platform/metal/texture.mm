#include "texture.hpp"
#include "core/global.hpp"
#include "graphics/texture.hpp"
#include "resources/texture/data.hpp"

namespace huedra {

void MetalTexture::init(id<MTLDevice> device, const TextureData& data)
{
    Texture::init(data.width, data.height, data.format, TextureType::COLOR);
    m_device = device;
    m_isRenderTargetTexture = false;

    m_textures.resize(0);
    MTLTextureDescriptor* desc = [[MTLTextureDescriptor alloc] init];
    desc.textureType = MTLTextureType2D;
    desc.pixelFormat = MTLPixelFormatRGBA8Unorm; // TODO: Find format
    desc.width = data.width;
    desc.height = data.height;
    desc.mipmapLevelCount = 1;
    desc.usage = MTLTextureUsageShaderRead;
    desc.storageMode = MTLStorageModePrivate;

    m_textures[0] = [m_device newTextureWithDescriptor:desc];
    // TODO: Transfer data to texture
}

void MetalTexture::init(id<MTLDevice> device, TextureType type, GraphicsDataFormat format, u32 width, u32 height,
                        u32 imageCount)
{
    Texture::init(width, height, format, type);
    m_device = device;
    m_isRenderTargetTexture = true;
    m_textures.resize(imageCount);

    MTLTextureDescriptor* desc = [[MTLTextureDescriptor alloc] init];
    desc.textureType = MTLTextureType2D;
    desc.pixelFormat = MTLPixelFormatRGBA8Unorm; // TODO: Find format
    desc.width = width;
    desc.height = height;
    desc.mipmapLevelCount = 1;
    desc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
    desc.storageMode = MTLStorageModePrivate;

    for (auto& texture : m_textures)
    {
        texture = [m_device newTextureWithDescriptor:desc];
    }
}

void MetalTexture::cleanup()
{
    for (auto& texture : m_textures)
    {
        [texture release];
    }
}

u32 MetalTexture::getIndex() const
{
    if (m_isRenderTargetTexture)
    {
        return global::graphicsManager.getCurrentFrame();
    }
    return 0;
}

} // namespace huedra