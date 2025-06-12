#include "context.hpp"
#include "core/global.hpp"
#include "core/log.hpp"
#include "graphics/pipeline_builder.hpp"
#include "graphics/pipeline_data.hpp"
#include "graphics/render_target.hpp"
#include "platform/cocoa/window.hpp"
#include "platform/metal/buffer.hpp"
#include "platform/metal/render_context.hpp"
#include "platform/metal/render_target.hpp"
#include "platform/metal/swapchain.hpp"
#include "platform/metal/texture.hpp"

#include <ranges>

namespace huedra {

void MetalContext::init()
{
    m_device = MTLCreateSystemDefaultDevice();
    if (m_device == nullptr)
    {
        log(LogLevel::ERR, "Could not create Metal device");
    }
    m_commandQueue = [m_device newCommandQueue];

    for (u32 i = 0; i < GraphicsManager::MAX_FRAMES_IN_FLIGHT; ++i)
    {
        m_inFlightSemaphores.push_back(dispatch_semaphore_create(1));
    }
}

void MetalContext::cleanup()
{
    waitIdle();

    for (auto& semaphore : m_inFlightSemaphores)
    {
        [semaphore release];
    }
    m_inFlightSemaphores.clear();

    for (auto& buffer : m_buffers)
    {
        buffer.cleanup();
    }
    m_buffers.clear();

    for (auto& texture : m_textures)
    {
        texture.cleanup();
    }
    m_textures.clear();

    for (auto& renderTarget : m_renderTargets)
    {
        renderTarget.cleanup();
    }
    m_renderTargets.clear();

    for (auto& info : m_samplers)
    {
        [info.sampler release];
    }
    m_samplers.clear();

    for (auto& swapchain : m_swapchains)
    {
        swapchain.cleanup();
    }
    m_swapchains.clear();

    [m_commandQueue release];
    [m_device release];
}

void MetalContext::createSwapchain(Window* window, bool renderDepth)
{
    MetalSwapchain& swapchain = m_swapchains.emplace_back();
    swapchain.init(m_device, m_commandQueue, static_cast<WindowCocoa*>(window), renderDepth);
}

void MetalContext::removeSwapchain(u64 index)
{
    m_swapchains[index].cleanup();
    m_swapchains.erase(m_swapchains.begin() + static_cast<i64>(index));
}

Buffer* MetalContext::createBuffer(BufferType type, BufferUsageFlags usage, u64 size, void* data)
{
    MetalBuffer& buffer = m_buffers.emplace_back();
    @autoreleasepool
    {
        if (type == BufferType::STATIC && data != nullptr)
        {
            id<MTLBuffer> stagingBuffer = [m_device newBufferWithBytes:data
                                                                length:size
                                                               options:MTLResourceStorageModeShared];
            buffer.init(m_device, type, size, usage);

            id<MTLCommandBuffer> cmd = [m_commandQueue commandBuffer];
            id<MTLBlitCommandEncoder> blit = [cmd blitCommandEncoder];
            [blit copyFromBuffer:stagingBuffer sourceOffset:0 toBuffer:buffer.get() destinationOffset:0 size:size];
            [blit endEncoding];
            [cmd commit];
        }
        else
        {
            buffer.init(m_device, type, size, usage, data);
        }
    }

    return &buffer;
}

Texture* MetalContext::createTexture(const TextureData& textureData)
{
    MetalTexture& texture = m_textures.emplace_back();
    texture.init(m_device, m_commandQueue, textureData);
    return &texture;
}

RenderTarget* huedra::MetalContext::createRenderTarget(RenderTargetType type, GraphicsDataFormat format, u32 width,
                                                       u32 height)
{
    MetalRenderTarget& renderTarget = m_renderTargets.emplace_back();
    renderTarget.init(m_device, type, format, width, height);
    return &renderTarget;
}

void MetalContext::removeBuffer(Buffer* buffer)
{
    auto it = std::ranges::find_if(m_buffers, [&](MetalBuffer& metalBuffer) { return &metalBuffer == buffer; });
    if (it != m_buffers.end())
    {
        it->cleanup();
        m_buffers.erase(it);
    }
}

void MetalContext::removeTexture(Texture* texture)
{
    auto it = std::ranges::find_if(m_textures, [&](MetalTexture& metalTexture) { return &metalTexture == texture; });
    if (it != m_textures.end())
    {
        it->cleanup();
        m_textures.erase(it);
    }
}

void MetalContext::removeRenderTarget(RenderTarget* renderTarget)
{
    auto it = std::ranges::find_if(
        m_renderTargets, [&](MetalRenderTarget& metalRenderTarget) { return &metalRenderTarget == renderTarget; });
    if (it != m_renderTargets.end())
    {
        it->cleanup();
        m_renderTargets.erase(it);
    }
}

void MetalContext::prepareSwapchains()
{
    for (auto& swapchain : m_swapchains)
    {
        swapchain.aquireNextDrawable();
    }
}

void MetalContext::setRenderGraph(RenderGraphBuilder& builder)
{
    if (m_curGraph.getHash() == builder.getHash())
    {
        return;
    }

    waitIdle();
    m_curGraph = builder;

    log(LogLevel::INFO, "New render graph with hash: 0x{:x}", m_curGraph.getHash());

    m_pipeline.initGraphics(builder.getRenderPasses().begin()->second.getPipeline(), m_device);
}

void MetalContext::render()
{
    @autoreleasepool
    {
        dispatch_semaphore_wait(m_inFlightSemaphores[global::graphicsManager.getCurrentFrame()], DISPATCH_TIME_FOREVER);

        id<MTLCommandBuffer> cmd = [m_commandQueue commandBuffer];
        [cmd addCompletedHandler:^(id<MTLCommandBuffer> _) {
          dispatch_semaphore_signal(m_inFlightSemaphores[global::graphicsManager.getCurrentFrame()]);
          for (auto& swapchain : m_swapchains)
          {
              swapchain.setTextureRendered();
          }
        }];

        for (auto& [name, info] : m_curGraph.getRenderPasses())
        {
            MTLRenderPassDescriptor* renderPassDesc = [MTLRenderPassDescriptor renderPassDescriptor];
            renderPassDesc.colorAttachments[0].texture =
                static_cast<MetalTexture*>(info.getRenderTargets().begin()->target->getColorTexture().get())->get();
            renderPassDesc.colorAttachments[0].loadAction = MTLLoadActionClear;
            renderPassDesc.colorAttachments[0].storeAction = MTLStoreActionStore;

            // Assumes all are using depth when first is
            if (info.getRenderTargets().begin()->target->usesDepth())
            {
                renderPassDesc.depthAttachment.texture =
                    static_cast<MetalTexture*>(info.getRenderTargets().begin()->target->getDepthTexture().get())->get();
                renderPassDesc.depthAttachment.loadAction = MTLLoadActionClear;
                renderPassDesc.depthAttachment.storeAction = MTLStoreActionStore;
            }

            vec3 clearColor = info.getRenderTargets().begin()->clearColor;
            renderPassDesc.colorAttachments[0].clearColor =
                MTLClearColorMake(clearColor.r, clearColor.g, clearColor.b, 1.0);

            id<MTLRenderCommandEncoder> encoder = [cmd renderCommandEncoderWithDescriptor:renderPassDesc];
            [encoder setRenderPipelineState:m_pipeline.get()];

            MetalRenderContext renderContext;
            renderContext.init(m_device, encoder, *this, m_pipeline);
            info.getCommands()(renderContext);

            [encoder endEncoding];
        }

        [cmd commit];
    }
}

id<MTLSamplerState> MetalContext::getSampler(const SamplerSettings& settings)
{
    for (auto& info : m_samplers)
    {
        if (info.settings == settings)
        {
            return info.sampler;
        }
    }

    @autoreleasepool
    {
        MTLSamplerDescriptor* samplerDesc = [[MTLSamplerDescriptor alloc] init];

        MTLSamplerMinMagFilter filter =
            settings.filter == SamplerFilter::LINEAR ? MTLSamplerMinMagFilterLinear : MTLSamplerMinMagFilterNearest;
        samplerDesc.minFilter = filter;
        samplerDesc.magFilter = filter;
        samplerDesc.mipFilter = MTLSamplerMipFilterNotMipmapped;

        constexpr auto convertAddressMode = [](SamplerAddressMode addressMode) -> MTLSamplerAddressMode {
            switch (addressMode)
            {
            case SamplerAddressMode::REPEAT:
                return MTLSamplerAddressModeRepeat;
            case SamplerAddressMode::MIRROR_REPEAT:
                return MTLSamplerAddressModeMirrorRepeat;
            case SamplerAddressMode::CLAMP_EDGE:
                return MTLSamplerAddressModeClampToEdge;
            case SamplerAddressMode::CLAMP_COLOR:
                return MTLSamplerAddressModeClampToBorderColor;
            default:
                return MTLSamplerAddressModeRepeat;
            };
            return MTLSamplerAddressModeRepeat;
        };

        samplerDesc.sAddressMode = convertAddressMode(settings.adressModeU);
        samplerDesc.tAddressMode = convertAddressMode(settings.adressModeV);
        samplerDesc.rAddressMode = convertAddressMode(settings.adressModeW);

        MTLSamplerBorderColor borderColor{MTLSamplerBorderColorOpaqueWhite};
        switch (settings.color)
        {
        case SamplerColor::WHITE:
            borderColor = MTLSamplerBorderColorOpaqueWhite;
        case SamplerColor::BLACK:
            borderColor = MTLSamplerBorderColorOpaqueBlack;
        case SamplerColor::ZERO_ALPHA:
            borderColor = MTLSamplerBorderColorTransparentBlack;
        };
        samplerDesc.borderColor = borderColor;
        samplerDesc.normalizedCoordinates = YES;
        samplerDesc.supportArgumentBuffers = YES;

        m_samplers.push_back({.settings = settings, .sampler = [m_device newSamplerStateWithDescriptor:samplerDesc]});
    }

    return m_samplers.back().sampler;
}

void MetalContext::waitIdle()
{
    // TODO: Improve this, should instead wait on commandBuffers to be done executing, probably easier to implement when
    // render graph architecture is in place
    for (auto& semaphore : m_inFlightSemaphores)
    {
        dispatch_semaphore_signal(semaphore);
    }
}

} // namespace huedra
