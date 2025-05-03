#pragma once

#include "graphics/render_context.hpp"
#include "platform/metal/config.hpp"
#include "platform/metal/pipeline.hpp"
#include <string_view>

namespace huedra {

class MetalRenderContext : public RenderContext
{
public:
    MetalRenderContext() = default;
    virtual ~MetalRenderContext() = default;

    MetalRenderContext(const MetalRenderContext& rhs) = delete;
    MetalRenderContext& operator=(const MetalRenderContext& rhs) = delete;
    MetalRenderContext(MetalRenderContext&& rhs) = delete;
    MetalRenderContext& operator=(MetalRenderContext&& rhs) = delete;

    void init(id<MTLRenderCommandEncoder> encoder, MetalPipeline& pipeline);
    void init(id<MTLComputeCommandEncoder> encoder, MetalPipeline& pipeline);

    void bindVertexBuffers(std::vector<Ref<Buffer>> buffers) override;
    void bindIndexBuffer(Ref<Buffer> buffer) override;
    void bindBuffer(Ref<Buffer> buffer, std::string_view name) override;
    void bindTexture(Ref<Texture> texture, std::string_view name) override;
    void bindSampler(const SamplerSettings& sampler, std::string_view name) override;

    void setParameter(void* data, u32 size, std::string_view name) override;

    void draw(u32 vertexCount, u32 instanceCount, u32 vertexOffset, u32 instanceOffset) override;
    void drawIndexed(u32 indexCount, u32 instanceCount, u32 indexOffset, u32 instanceOffset) override;
    void dispatch(u32 groupX, u32 groupY, u32 groupZ) override;

private:
    id<MTLRenderCommandEncoder> m_encoder;
    id<MTLComputeCommandEncoder> m_computeEncoder;

    MetalPipeline* m_pipeline{nullptr};
    id<MTLBuffer> m_boundIndexBuffer{nil};
};

} // namespace huedra