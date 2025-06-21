#pragma once

#include "graphics/render_context.hpp"
#include "platform/metal/config.hpp"
#include "platform/metal/context.hpp"
#include "platform/metal/parameter_handler.hpp"
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

    void init(id<MTLDevice> device, id<MTLRenderCommandEncoder> encoder, MetalContext& context,
              MetalPipeline& pipeline);
    void init(id<MTLComputeCommandEncoder> encoder, MetalContext& context, MetalPipeline& pipeline);

    void bindVertexBuffers(std::vector<Ref<Buffer>> buffers) override;
    void bindIndexBuffer(Ref<Buffer> buffer) override;
    void bindBuffer(Ref<Buffer> buffer, std::string_view name) override;
    void bindTexture(Ref<Texture> texture, std::string_view name) override;
    void bindSampler(const SamplerSettings& sampler, std::string_view name) override;

    void setParameter(void* data, u32 size, std::string_view name) override;

    void draw(u32 vertexCount, u32 instanceCount, u32 vertexOffset, u32 instanceOffset) override;
    void drawIndexed(u32 indexCount, u32 instanceCount, u32 indexOffset, u32 instanceOffset) override;
    void dispatchGroups(u32 groupX, u32 groupY, u32 groupZ) override;
    void dispatch(u32 x, u32 y, u32 z) override;

    uvec3 getComputeThreadsPerGroup() override { return m_pipeline->getShaderModule().getComputeThreadsPerGroup(); };

private:
    id<MTLRenderCommandEncoder> m_encoder;
    id<MTLComputeCommandEncoder> m_computeEncoder;

    MetalContext* m_context{nullptr};
    MetalPipeline* m_pipeline{nullptr};
    MetalParameterHandler* m_parameterHandler{nullptr};
    id<MTLBuffer> m_boundIndexBuffer{nil};
};

} // namespace huedra