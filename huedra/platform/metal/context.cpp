#include "context.hpp"

namespace huedra {

void MetalContext::init() {}

void MetalContext::cleanup() {}

void MetalContext::createSwapchain(Window* window, bool renderDepth) {}

void MetalContext::removeSwapchain(u64 index) {}

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

void MetalContext::render() {}

} // namespace huedra
