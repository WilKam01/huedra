#pragma once

#include "graphics/context.hpp"

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

    virtual void init() override;
    virtual void cleanup() override;

    virtual void createSwapchain(Window* window, bool renderDepth) override;
    virtual void removeSwapchain(u64 index) override;

    virtual Buffer* createBuffer(BufferType type, BufferUsageFlags usage, u64 size, void* data) override;
    virtual Texture* createTexture(const TextureData& textureData) override;
    virtual RenderTarget* createRenderTarget(RenderTargetType type, GraphicsDataFormat format, u32 width,
                                             u32 height) override;

    virtual void removeBuffer(Buffer* buffer) override;
    virtual void removeTexture(Texture* texture) override;
    virtual void removeRenderTarget(RenderTarget* renderTarget) override;

    virtual void prepareSwapchains() override;
    virtual void setRenderGraph(RenderGraphBuilder& builder) override;
    virtual void render() override;

private:
};

} // namespace huedra