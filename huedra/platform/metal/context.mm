#include "context.hpp"
#include "core/global.hpp"
#include "core/log.hpp"
#include "platform/cocoa/window.hpp"
#include <Metal/Metal.h>
#include <QuartzCore/QuartzCore.h>

namespace huedra {

void MetalContext::init()
{
    m_device = MTLCreateSystemDefaultDevice();
    if (m_device == nullptr)
    {
        log(LogLevel::ERR, "Could not create Metal device");
    }
    m_commandQueue = [m_device newCommandQueue];

    // Create pipeline state
    NSError* error = nil;
    NSString* source = [NSString stringWithContentsOfFile:@"assets/shaders/shader.metal"
                                                 encoding:NSUTF8StringEncoding
                                                    error:&error];

    id<MTLLibrary> library = [m_device newLibraryWithSource:source options:nil error:&error];
    id<MTLFunction> vert = [library newFunctionWithName:@"vertex_main"];
    id<MTLFunction> frag = [library newFunctionWithName:@"fragment_main"];

    MTLRenderPipelineDescriptor* desc = [[MTLRenderPipelineDescriptor alloc] init];
    desc.vertexFunction = vert;
    desc.fragmentFunction = frag;
    desc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;

    m_pipelineState = [m_device newRenderPipelineStateWithDescriptor:desc error:&error];
    if (m_pipelineState == nullptr)
    {
        log(LogLevel::ERR, "Failed to create pipeline state: {}", [[error localizedDescription] UTF8String]);
        return;
    }
}

void MetalContext::cleanup()
{
    for (auto& layer : m_windowLayers)
    {
        [layer release];
    }
    [m_commandQueue release];
    [m_device release];
}

void MetalContext::createSwapchain(Window* window, bool renderDepth)
{
    auto* cocoaWindow = static_cast<WindowCocoa*>(window);
    CAMetalLayer* layer = [CAMetalLayer layer];
    layer.device = m_device;
    layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    layer.framebufferOnly = NO;
    layer.contentsScale = cocoaWindow->getScreenDPI();
    layer.frame = cocoaWindow->get().contentView.bounds;
    layer.maximumDrawableCount = GraphicsManager::MAX_FRAMES_IN_FLIGHT;
    [cocoaWindow->get().contentView setLayer:layer];
    [cocoaWindow->get().contentView setWantsLayer:YES];
    m_windowLayers.push_back(layer);
}

void MetalContext::removeSwapchain(u64 index)
{
    [m_windowLayers[index] release];
    m_windowLayers.erase(m_windowLayers.begin() + static_cast<i64>(index));
}

Buffer* MetalContext::createBuffer(BufferType type, BufferUsageFlags usage, u64 size, void* data) { return nullptr; }

Texture* MetalContext::createTexture(const TextureData& textureData) { return nullptr; }

RenderTarget* huedra::MetalContext::createRenderTarget(RenderTargetType type, GraphicsDataFormat format, u32 width,
                                                       u32 height)
{
    return nullptr;
}

void MetalContext::removeBuffer(Buffer* buffer) {}

void MetalContext::removeTexture(Texture* texture) {}

void MetalContext::removeRenderTarget(RenderTarget* renderTarget) {}

void MetalContext::prepareSwapchains() {}

void MetalContext::setRenderGraph(RenderGraphBuilder& builder) {}

void MetalContext::render()
{
    id<CAMetalDrawable> drawable = [m_windowLayers[0] nextDrawable];
    if (drawable == nullptr)
    {
        return;
    }

    id<MTLCommandBuffer> cmd = [m_commandQueue commandBuffer];

    MTLRenderPassDescriptor* renderPassDesc = [MTLRenderPassDescriptor renderPassDescriptor];
    renderPassDesc.colorAttachments[0].texture = drawable.texture;
    renderPassDesc.colorAttachments[0].loadAction = MTLLoadActionClear;
    renderPassDesc.colorAttachments[0].storeAction = MTLStoreActionStore;
    renderPassDesc.colorAttachments[0].clearColor = MTLClearColorMake(0.2, 0.2, 0.2, 1.0);

    id<MTLRenderCommandEncoder> encoder = [cmd renderCommandEncoderWithDescriptor:renderPassDesc];
    [encoder setRenderPipelineState:m_pipelineState];
    [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
    [encoder endEncoding];

    [cmd presentDrawable:drawable];
    [cmd commit];
}

} // namespace huedra
