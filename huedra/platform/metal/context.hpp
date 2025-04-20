#pragma once

#include "graphics/context.hpp"

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#include <vector>

namespace huedra {

class MetalContext : public GraphicalContext
{

public:
    MetalContext() = default;
    virtual ~MetalContext() = default;

    MetalContext(const MetalContext& rhs) = delete;
    MetalContext& operator=(const MetalContext& rhs) = delete;
    MetalContext(MetalContext&& rhs) = delete;
    MetalContext& operator=(MetalContext&& rhs) = delete;

    void init() override;
    void cleanup() override;

    void createSwapchain(Window* window, bool renderDepth) override;
    void removeSwapchain(u64 index) override;

    Buffer* createBuffer(BufferType type, BufferUsageFlags usage, u64 size, void* data) override;
    Texture* createTexture(const TextureData& textureData) override;
    RenderTarget* createRenderTarget(RenderTargetType type, GraphicsDataFormat format, u32 width, u32 height) override;

    void removeBuffer(Buffer* buffer) override;
    void removeTexture(Texture* texture) override;
    void removeRenderTarget(RenderTarget* renderTarget) override;

    void prepareSwapchains() override;
    void setRenderGraph(RenderGraphBuilder& builder) override;
    void render() override;

private:
    id<MTLDevice> m_device;
    id<MTLCommandQueue> m_commandQueue;
    std::vector<CAMetalLayer*> m_windowLayers;

    id<MTLRenderPipelineState> m_pipelineState;
};

} // namespace huedra