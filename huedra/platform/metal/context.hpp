#pragma once

#include "graphics/context.hpp"
#include "graphics/render_graph_builder.hpp"
#include "platform/metal/buffer.hpp"
#include "platform/metal/config.hpp"
#include "platform/metal/pipeline.hpp"
#include "platform/metal/render_target.hpp"
#include "platform/metal/swapchain.hpp"
#include "platform/metal/texture.hpp"

#include <array>
#include <deque>
#include <dispatch/dispatch.h>

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

    id<MTLSamplerState> getSampler(const SamplerSettings& settings);

private:
    void waitIdle();

    id<MTLDevice> m_device;
    id<MTLCommandQueue> m_commandQueue;
    std::deque<MetalSwapchain> m_swapchains;
    std::deque<MetalBuffer> m_buffers;
    std::deque<MetalTexture> m_textures;
    std::deque<MetalRenderTarget> m_renderTargets;

    struct SamplerInfo
    {
        SamplerSettings settings;
        id<MTLSamplerState> sampler{nullptr};
    };
    std::vector<SamplerInfo> m_samplers;

    std::vector<dispatch_semaphore_t> m_inFlightSemaphores;

    RenderGraphBuilder m_curGraph;

    MetalPipeline m_pipeline;
};

} // namespace huedra