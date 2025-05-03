#include "render_context.hpp"
#include "core/log.hpp"
#include "platform/metal/buffer.hpp"
#include "platform/metal/render_target.hpp"
#include <Metal/Metal.h>

namespace huedra {

void MetalRenderContext::init(id<MTLRenderCommandEncoder> encoder, MetalPipeline& pipeline)
{
    m_encoder = encoder;
    m_computeEncoder = nil;
    m_pipeline = &pipeline;
    m_boundIndexBuffer = nil;
}

void MetalRenderContext::init(id<MTLComputeCommandEncoder> encoder, MetalPipeline& pipeline)
{
    m_encoder = nil;
    m_computeEncoder = encoder;
    m_pipeline = &pipeline;
    m_boundIndexBuffer = nil;
}

void MetalRenderContext::bindVertexBuffers(std::vector<Ref<Buffer>> buffers)
{
    if (m_pipeline->getBuilder().getType() != PipelineType::GRAPHICS)
    {
        log(LogLevel::WARNING, "Could not bind vertex buffers, not using a graphics pipeline");
        return;
    }

    std::vector<id<MTLBuffer>> metalBuffers(buffers.size());
    for (u64 i = 0; i < buffers.size(); ++i)
    {
        if (!buffers[i].valid())
        {
            log(LogLevel::WARNING, "Could not bind vertex buffer: {}. Not valid", i);
            return;
        }
        if ((buffers[i]->getBufferUsage() & HU_BUFFER_USAGE_VERTEX_BUFFER) == 0)
        {
            log(LogLevel::WARNING, "Could not bind vertex buffer: {}. Buffer usage flag vertex buffer not set", i);
            return;
        }
        // metalBuffers[i] = static_cast<MetalBuffer*>(buffers[i]->get());
    }
}

void MetalRenderContext::bindIndexBuffer(Ref<Buffer> buffer)
{
    if (m_pipeline->getBuilder().getType() != PipelineType::GRAPHICS)
    {
        log(LogLevel::WARNING, "Could not bind index buffer, not using a graphics pipeline");
        return;
    }

    if (!buffer.valid())
    {
        log(LogLevel::WARNING, "Could not bind index buffer. Not valid");
        return;
    }

    if ((buffer->getBufferUsage() & HU_BUFFER_USAGE_INDEX_BUFFER) == 0)
    {
        log(LogLevel::WARNING, "Could not bind index buffer. Buffer usage flag index buffer not set");
        return;
    }

    // m_boundIndexBuffer = buffer->get();
}

void MetalRenderContext::bindBuffer(Ref<Buffer> buffer, std::string_view name)
{
    if (!buffer.valid())
    {
        log(LogLevel::WARNING, "Could not bind buffer, reference invalid");
        return;
    }
}

void MetalRenderContext::bindTexture(Ref<Texture> texture, std::string_view name)
{
    if (!texture.valid())
    {
        log(LogLevel::WARNING, "Could not bind texture, reference invalid");
        return;
    }
}

void MetalRenderContext::bindSampler(const SamplerSettings& sampler, std::string_view name) {}

void MetalRenderContext::setParameter(void* data, u32 size, std::string_view name) {}

void MetalRenderContext::draw(u32 vertexCount, u32 instanceCount, u32 vertexOffset, u32 instanceOffset)
{
    if (m_pipeline->getBuilder().getType() != PipelineType::GRAPHICS)
    {
        log(LogLevel::WARNING, "Could not execute draw command, not using a graphics pipeline");
        return;
    }

    [m_encoder drawPrimitives:MTLPrimitiveTypeTriangleStrip
                  vertexStart:vertexOffset
                  vertexCount:vertexCount
                instanceCount:instanceCount
                 baseInstance:instanceOffset];
}

void MetalRenderContext::drawIndexed(u32 indexCount, u32 instanceCount, u32 indexOffset, u32 instanceOffset)
{
    if (m_pipeline->getBuilder().getType() != PipelineType::GRAPHICS)
    {
        log(LogLevel::WARNING, "Could not execute drawIndexed call, not using a graphics pipeline");
        return;
    }

    if (m_boundIndexBuffer != nil)
    {
        log(LogLevel::WARNING, "Could not execute drawIndexed command, no index buffer has been bound");
        return;
    }

    [m_encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangleStrip
                          indexCount:indexCount
                           indexType:MTLIndexTypeUInt32
                         indexBuffer:m_boundIndexBuffer
                   indexBufferOffset:indexOffset
                       instanceCount:instanceCount
                          baseVertex:0
                        baseInstance:instanceOffset];
}

void MetalRenderContext::dispatch(u32 groupX, u32 groupY, u32 groupZ)
{
    if (m_pipeline->getBuilder().getType() != PipelineType::COMPUTE)
    {
        log(LogLevel::WARNING, "Could not execute dispatch call, not using a compute pipeline");
        return;
    }

    [m_computeEncoder dispatchThreadgroups:MTLSizeMake(1, 1, 1)
                     threadsPerThreadgroup:MTLSizeMake(groupX, groupY, groupZ)];
}

} // namespace huedra