#include "context.hpp"
#include "core/global.hpp"
#include "core/log.hpp"
#include "graphics/pipeline_builder.hpp"
#include "graphics/pipeline_data.hpp"
#include "graphics/render_pass_builder.hpp"
#include "graphics/render_target.hpp"
#include "platform/cocoa/window.hpp"
#include "platform/metal/buffer.hpp"
#include "platform/metal/render_context.hpp"
#include "platform/metal/render_target.hpp"
#include "platform/metal/swapchain.hpp"
#include "platform/metal/texture.hpp"

#include <atomic>
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
        m_batchSharedEvents.push_back([m_device newSharedEvent]);
    }
}

void MetalContext::cleanup()
{
    waitIdle();

    for (auto& sharedEvent : m_batchSharedEvents)
    {
        [sharedEvent release];
    }
    m_batchSharedEvents.clear();

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

    for (auto& batch : m_passBatches)
    {
        for (auto& pass : batch.passes)
        {
            pass.pipeline.cleanup();
        }
    }
    m_passBatches.clear();

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

    for (auto& batch : m_passBatches)
    {
        for (auto& pass : batch.passes)
        {
            pass.pipeline.cleanup();
        }
    }
    m_passBatches.clear();

    for (auto& info : m_samplers)
    {
        [info.sampler release];
    }
    m_samplers.clear();

    // Create render passes
    struct VersionData
    {
        u32 version{0};
    };
    std::map<void*, VersionData> resourceVersions; // Keeping track on resource iterations

    m_passBatches.emplace_back(GraphicsManager::MAX_FRAMES_IN_FLIGHT); // At least one batch
    for (const auto& key : m_curGraph.getRenderPassNames())
    {
        const RenderPassBuilder& info = m_curGraph.getRenderPass(key);
        PassInfo passInfo;
        if (info.getType() == RenderPassType::GRAPHICS)
        {
            passInfo.pipeline.initGraphics(m_device, info.getPipeline(), info.getRenderTargets(),
                                           info.getRenderTargetUse());
        }
        else if (info.getType() == RenderPassType::COMPUTE)
        {
            passInfo.pipeline.initCompute(m_device, info.getPipeline());
        }
        passInfo.commands = info.getCommands();
        passInfo.clearTargets = info.getClearRenderTargets();
        passInfo.renderTargetUse = info.getRenderTargetUse();

        std::vector<MetalSwapchain*> swapchains;
        u32 latestVersion = 0; // Checking the latest iteration of an input resource

        // Inputs
        for (auto& input : info.getInputs())
        {
            void* ptr = input.type == RenderPassReference::Type::BUFFER ? static_cast<void*>(input.buffer)
                                                                        : static_cast<void*>(input.texture);
            if (!resourceVersions.contains(ptr))
            {
                resourceVersions.insert(std::pair<void*, u32>(ptr, {}));
            }
            latestVersion = std::max(latestVersion, resourceVersions[ptr].version);

            if (input.type == RenderPassReference::Type::TEXTURE)
            {
                auto* texture = static_cast<MetalTexture*>(input.texture);
                if (texture->getRenderTarget() != nullptr && texture->getRenderTarget()->getSwapchain() != nullptr)
                {
                    swapchains.push_back(texture->getRenderTarget()->getSwapchain());
                    m_activeSwapchains.insert(texture->getRenderTarget()->getSwapchain());
                }
            }
        }

        // Render target (outputs for textures (color or depth or both))
        // Also input for textures if not clearing contents
        for (auto& targetInfo : info.getRenderTargets())
        {
            auto* renderTarget = static_cast<MetalRenderTarget*>(targetInfo.target.get());
            if (renderTarget->usesColor() && (info.getRenderTargetUse() == RenderTargetType::COLOR_AND_DEPTH ||
                                              info.getRenderTargetUse() == RenderTargetType::COLOR))
            {
                void* ptr = static_cast<void*>(renderTarget->getColorTexture().get());
                if (!resourceVersions.contains(ptr))
                {
                    resourceVersions.insert(std::pair<void*, u32>(ptr, {}));
                }
                if (!passInfo.clearTargets)
                {
                    latestVersion = std::max(latestVersion, resourceVersions[ptr].version);
                }
                resourceVersions[ptr].version = latestVersion + 1;
            }

            if (renderTarget->usesDepth() && (info.getRenderTargetUse() == RenderTargetType::COLOR_AND_DEPTH ||
                                              info.getRenderTargetUse() == RenderTargetType::DEPTH))
            {
                void* ptr = static_cast<void*>(renderTarget->getDepthTexture().get());
                if (!resourceVersions.contains(ptr))
                {
                    resourceVersions.insert(std::pair<void*, u32>(ptr, {}));
                }
                if (!passInfo.clearTargets)
                {
                    latestVersion = std::max(latestVersion, resourceVersions[ptr].version);
                }
                resourceVersions[ptr].version = latestVersion + 1;
            }

            if (renderTarget->getSwapchain() != nullptr)
            {
                swapchains.push_back(renderTarget->getSwapchain());
                m_activeSwapchains.insert(renderTarget->getSwapchain());
            }

            passInfo.renderTargets.push_back(renderTarget);
            passInfo.clearColors.push_back(targetInfo.clearColor);
        }

        // Outputs
        for (auto& output : info.getOutputs())
        {
            void* ptr = output.type == RenderPassReference::Type::BUFFER ? static_cast<void*>(output.buffer)
                                                                         : static_cast<void*>(output.texture);
            if (!resourceVersions.contains(ptr))
            {
                resourceVersions.insert(std::pair<void*, u32>(ptr, {}));
            }
            resourceVersions[ptr].version = latestVersion + 1;

            if (output.type == RenderPassReference::Type::TEXTURE)
            {
                auto* texture = static_cast<MetalTexture*>(output.texture);
                if (texture->getRenderTarget() != nullptr && texture->getRenderTarget()->getSwapchain() != nullptr)
                {
                    swapchains.push_back(texture->getRenderTarget()->getSwapchain());
                    m_activeSwapchains.insert(texture->getRenderTarget()->getSwapchain());
                }
            }
        }

        if (m_passBatches.size() <= latestVersion)
        {
            for (u64 i = m_passBatches.size(); i < latestVersion + 1; ++i)
            {
                m_passBatches.emplace_back(GraphicsManager::MAX_FRAMES_IN_FLIGHT);
            }
        }
        m_passBatches[latestVersion].passes.push_back(passInfo);
    }
}

void MetalContext::render()
{
    @autoreleasepool
    {
        dispatch_semaphore_wait(m_inFlightSemaphores[global::graphicsManager.getCurrentFrame()], DISPATCH_TIME_FOREVER);

        for (u32 i = 0; i < m_passBatches.size(); ++i)
        {
            std::atomic_uint& cmdsLeft = m_passBatches[i].cmdsLeft[global::graphicsManager.getCurrentFrame()];
            cmdsLeft.store(m_passBatches[i].passes.size());

            for (u32 j = 0; j < m_passBatches[i].passes.size(); ++j)
            {
                PassInfo& pass = m_passBatches[i].passes[j];
                id<MTLCommandBuffer> cmd = [m_commandQueue commandBuffer];

                // Not first batch
                if (i > 0)
                {
                    [cmd encodeWaitForEvent:m_batchSharedEvents[global::graphicsManager.getCurrentFrame()] value:i - 1];
                }

                if (pass.pipeline.getBuilder().getType() == PipelineType::GRAPHICS)
                {
                    MTLRenderPassDescriptor* renderPassDesc = [MTLRenderPassDescriptor renderPassDescriptor];
                    if (pass.renderTargetUse == RenderTargetType::COLOR_AND_DEPTH ||
                        pass.renderTargetUse == RenderTargetType::COLOR)
                    {

                        for (u32 i = 0; i < pass.renderTargets.size(); ++i)
                        {
                            renderPassDesc.colorAttachments[i].texture =
                                pass.renderTargets[i]->getMetalColorTexture().get();
                            renderPassDesc.colorAttachments[i].loadAction =
                                pass.clearTargets ? MTLLoadActionClear : MTLLoadActionLoad;
                            renderPassDesc.colorAttachments[i].storeAction = MTLStoreActionStore;

                            vec3 clearColor = pass.clearColors[i];
                            renderPassDesc.colorAttachments[0].clearColor =
                                MTLClearColorMake(clearColor.r, clearColor.g, clearColor.b, 1.0);
                        }
                    }

                    if (pass.renderTargetUse == RenderTargetType::COLOR_AND_DEPTH ||
                        pass.renderTargetUse == RenderTargetType::DEPTH)
                    {
                        renderPassDesc.depthAttachment.texture =
                            pass.renderTargets.front()->getMetalDepthTexture().get();
                        renderPassDesc.depthAttachment.loadAction =
                            pass.clearTargets ? MTLLoadActionClear : MTLLoadActionLoad;
                        renderPassDesc.depthAttachment.storeAction = MTLStoreActionStore;
                    }

                    id<MTLRenderCommandEncoder> encoder = [cmd renderCommandEncoderWithDescriptor:renderPassDesc];
                    [encoder setRenderPipelineState:pass.pipeline.getGraphicsPipeline()];

                    MetalRenderContext renderContext;
                    renderContext.init(m_device, encoder, *this, pass.pipeline, pass.renderTargetUse);
                    pass.commands(renderContext);

                    [encoder endEncoding];
                }
                else if (pass.pipeline.getBuilder().getType() == PipelineType::COMPUTE)
                {
                    id<MTLComputeCommandEncoder> encoder =
                        [cmd computeCommandEncoderWithDispatchType:MTLDispatchTypeConcurrent];
                    [encoder setComputePipelineState:pass.pipeline.getComputePipeline()];

                    MetalRenderContext renderContext;
                    renderContext.init(encoder, *this, pass.pipeline);
                    pass.commands(renderContext);

                    [encoder endEncoding];
                }

                // Last batch
                if (i == m_passBatches.size() - 1)
                {
                    [cmd addCompletedHandler:^(id<MTLCommandBuffer> _) {
                      if (cmdsLeft.fetch_sub(1) - 1 == 0) // -1 since it returns previous value
                      {
                          dispatch_semaphore_signal(m_inFlightSemaphores[global::graphicsManager.getCurrentFrame()]);
                          for (auto* swapchain : m_activeSwapchains)
                          {
                              swapchain->setTextureRendered();
                          }
                      }
                    }];
                }
                // Only one pass in batch (can signal event directly instead of waiting on other command buffers)
                else if (m_passBatches[i].passes.size() == 1)
                {
                    [cmd encodeSignalEvent:m_batchSharedEvents[global::graphicsManager.getCurrentFrame()] value:i];
                }
                else
                {
                    [cmd addCompletedHandler:^(id<MTLCommandBuffer> _) {
                      if (cmdsLeft.fetch_sub(1) - 1 == 0) // -1 since it returns previous value
                      {
                          id<MTLCommandBuffer> signalCmd = [m_commandQueue commandBuffer];
                          [signalCmd encodeSignalEvent:m_batchSharedEvents[global::graphicsManager.getCurrentFrame()]
                                                 value:i];
                          [signalCmd commit];
                      }
                    }];
                }

                [cmd commit];
            }
        }
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
