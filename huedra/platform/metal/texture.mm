#include "texture.hpp"
#include "core/global.hpp"
#include "graphics/texture.hpp"
#include "platform/metal/render_target.hpp"
#include "platform/metal/type_converter.hpp"
#include "resources/texture/data.hpp"

namespace huedra {

void MetalTexture::init(id<MTLDevice> device, id<MTLCommandQueue> commandQueue, const TextureData& data)
{
    @autoreleasepool
    {
        Texture::init(data.width, data.height, data.format, TextureType::COLOR);
        m_device = device;
        m_renderTarget = nullptr;

        m_textures.resize(1);
        MTLTextureDescriptor* desc = [[MTLTextureDescriptor alloc] init];
        desc.textureType = MTLTextureType2D;
        desc.pixelFormat = converter::convertPixelDataFormat(data.format);
        desc.width = data.width;
        desc.height = data.height;
        desc.mipmapLevelCount = 1;
        desc.usage = MTLTextureUsageShaderRead;
        desc.storageMode = MTLStorageModePrivate;

        m_textures[0] = [m_device newTextureWithDescriptor:desc];

        // Create staging texture
        desc.storageMode = MTLStorageModeShared;
        desc.usage = MTLTextureUsageShaderRead | MTLTextureUsagePixelFormatView;

        id<MTLTexture> stagingTexture = [device newTextureWithDescriptor:desc];
        MTLRegion region = {{0, 0, 0}, {data.width, data.height, 1}};

        [stagingTexture replaceRegion:region
                          mipmapLevel:0
                            withBytes:data.texels.data()
                          bytesPerRow:static_cast<NSUInteger>(data.width * data.texelSize)];

        id<MTLCommandBuffer> cmd = [commandQueue commandBuffer];
        id<MTLBlitCommandEncoder> blitEncoder = [cmd blitCommandEncoder];

        [blitEncoder copyFromTexture:stagingTexture
                         sourceSlice:0
                         sourceLevel:0
                        sourceOrigin:{0, 0, 0}
                          sourceSize:{data.width, data.height, 1}
                           toTexture:m_textures[0]
                    destinationSlice:0
                    destinationLevel:0
                   destinationOrigin:{0, 0, 0}];

        [blitEncoder endEncoding];
        [cmd commit];
        [cmd waitUntilCompleted];
        [stagingTexture release];
    }
}

void MetalTexture::init(id<MTLDevice> device, MetalRenderTarget* renderTarget, TextureType type,
                        GraphicsDataFormat format, u32 width, u32 height, u32 imageCount)
{
    @autoreleasepool
    {
        Texture::init(width, height, format, type);
        m_device = device;
        m_renderTarget = renderTarget;
        m_textures.resize(imageCount);

        MTLTextureDescriptor* desc = [[MTLTextureDescriptor alloc] init];
        desc.textureType = MTLTextureType2D;
        desc.pixelFormat =
            type == TextureType::COLOR ? converter::convertPixelDataFormat(format) : MTLPixelFormatDepth32Float;
        desc.width = width;
        desc.height = height;
        desc.mipmapLevelCount = 1;
        desc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite;
        desc.storageMode = MTLStorageModePrivate;

        for (auto& texture : m_textures)
        {
            texture = [m_device newTextureWithDescriptor:desc];
        }
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
    if (m_renderTarget != nullptr)
    {
        return global::graphicsManager.getCurrentFrame();
    }
    return 0;
}

} // namespace huedra