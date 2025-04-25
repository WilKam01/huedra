#include "context.hpp"
#include "core/global.hpp"
#include "core/log.hpp"
#include "graphics/pipeline_builder.hpp"
#include "graphics/pipeline_data.hpp"
#include "graphics/render_target.hpp"
#include "platform/cocoa/window.hpp"
#include "platform/metal/buffer.hpp"
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
}

void MetalContext::cleanup()
{
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
    swapchain.init(m_device, static_cast<WindowCocoa*>(window), renderDepth);
}

void MetalContext::removeSwapchain(u64 index)
{
    m_swapchains[index].cleanup();
    m_swapchains.erase(m_swapchains.begin() + static_cast<i64>(index));
}

Buffer* MetalContext::createBuffer(BufferType type, BufferUsageFlags usage, u64 size, void* data)
{
    MetalBuffer& buffer = m_buffers.emplace_back();
    buffer.init(m_device, type, size, usage, data);
    return &buffer;
}

Texture* MetalContext::createTexture(const TextureData& textureData)
{
    MetalTexture& texture = m_textures.emplace_back();
    texture.init(m_device, textureData);
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

    m_curGraph = builder;

    log(LogLevel::INFO, "New render graph with hash: 0x{:x}", m_curGraph.getHash());

    m_pipeline.initGraphics(builder.getRenderPasses().begin()->second.getPipeline(), m_device);
}

void MetalContext::render()
{
    id<CAMetalDrawable> drawable = [m_swapchains[0].getLayer() nextDrawable];
    if (drawable == nullptr)
    {
        return;
    }

    id<MTLCommandBuffer> cmd = [m_commandQueue commandBuffer];

    MTLRenderPassDescriptor* renderPassDesc = [MTLRenderPassDescriptor renderPassDescriptor];
    renderPassDesc.colorAttachments[0].texture = drawable.texture;
    renderPassDesc.colorAttachments[0].loadAction = MTLLoadActionClear;
    renderPassDesc.colorAttachments[0].storeAction = MTLStoreActionStore;
    renderPassDesc.colorAttachments[0].clearColor = MTLClearColorMake(0.1, 0.1, 0.1, 1.0);

    id<MTLRenderCommandEncoder> encoder = [cmd renderCommandEncoderWithDescriptor:renderPassDesc];
    [encoder setRenderPipelineState:m_pipeline.get()];
    [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
    [encoder endEncoding];

    [cmd presentDrawable:drawable];
    [cmd commit];

    [encoder release];
    [cmd release];
    [drawable release];
}

} // namespace huedra
