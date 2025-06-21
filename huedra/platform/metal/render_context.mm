#include "render_context.hpp"
#include "core/log.hpp"
#include "graphics/pipeline_data.hpp"
#include "graphics/texture.hpp"
#include "platform/metal/buffer.hpp"
#include "platform/metal/render_target.hpp"
#include "platform/metal/texture.hpp"
#include <Foundation/Foundation.h>
#include <Metal/MTLRenderCommandEncoder.h>
#include <Metal/Metal.h>
#include <objc/NSObjCRuntime.h>

namespace huedra {

void MetalRenderContext::init(id<MTLDevice> device, id<MTLRenderCommandEncoder> encoder, MetalContext& context,
                              MetalPipeline& pipeline)
{
    m_encoder = encoder;
    m_computeEncoder = nil;
    m_context = &context;
    m_pipeline = &pipeline;
    m_parameterHandler = &m_pipeline->getParameterHandler();
    m_parameterHandler->resetIndices();
    m_boundIndexBuffer = nil;

    @autoreleasepool
    {
        [encoder setFrontFacingWinding:MTLWindingCounterClockwise];
        [encoder setCullMode:MTLCullModeBack];

        MTLDepthStencilDescriptor* depthDesc = [[MTLDepthStencilDescriptor alloc] init];
        depthDesc.depthCompareFunction = MTLCompareFunctionLess;
        depthDesc.depthWriteEnabled = YES;

        id<MTLDepthStencilState> depthState = [device newDepthStencilStateWithDescriptor:depthDesc];
        [encoder setDepthStencilState:depthState];
    }
}

void MetalRenderContext::init(id<MTLComputeCommandEncoder> encoder, MetalContext& context, MetalPipeline& pipeline)
{
    m_encoder = nil;
    m_computeEncoder = encoder;
    m_context = &context;
    m_pipeline = &pipeline;
    m_parameterHandler = &m_pipeline->getParameterHandler();
    m_parameterHandler->resetIndices();
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
    std::vector<NSUInteger> offsets(buffers.size(), 0);
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
        metalBuffers[i] = static_cast<MetalBuffer*>(buffers[i].get())->get();
    }
    [m_encoder
        setVertexBuffers:metalBuffers.data()
                 offsets:offsets.data()
               withRange:NSMakeRange(m_pipeline->getShaderModule().getVertexBufferOffsetMetal(), buffers.size())];
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

    m_boundIndexBuffer = static_cast<MetalBuffer*>(buffer.get())->get();
}

void MetalRenderContext::bindBuffer(Ref<Buffer> buffer, std::string_view name)
{
    if (!buffer.valid())
    {
        log(LogLevel::WARNING, "Could not bind buffer, reference invalid");
        return;
    }

    std::optional<ResourcePosition> resource = m_pipeline->getShaderModule().getResource(name);
    if (!resource.has_value())
    {
        log(LogLevel::WARNING, "Could not bind buffer, no resource named \"{}\"", name);
        return;
    }

    if (resource.value().info.type != ResourceType::CONSTANT_BUFFER &&
        resource.value().info.type != ResourceType::STRUCTURED_BUFFER)
    {
        log(LogLevel::WARNING, "Could not bind buffer, \"{}\" is a {}", name,
            ResourceTypeNames[static_cast<u32>(resource.value().info.type)]);
        return;
    }

    @autoreleasepool
    {

        if (resource->info.partOfArgBuffer)
        {
            m_parameterHandler->writeBuffer(*static_cast<MetalBuffer*>(buffer.get()), resource.value().set,
                                            resource.value().binding);

            MTLResourceUsage usage = MTLResourceUsageRead;
            if (resource.value().info.type != ResourceType::STRUCTURED_BUFFER)
            {
                usage |= MTLResourceUsageWrite;
            }

            if (m_pipeline->getBuilder().getType() == PipelineType::GRAPHICS)
            {
                MTLRenderStages stages =
                    resource->info.shaderStage == ShaderStage::FRAGMENT ? MTLRenderStageFragment : MTLRenderStageVertex;
                [m_encoder useResource:static_cast<MetalBuffer*>(buffer.get())->get() usage:usage stages:stages];
            }
            else if (m_pipeline->getBuilder().getType() == PipelineType::COMPUTE)
            {
                [m_computeEncoder useResource:static_cast<MetalBuffer*>(buffer.get())->get() usage:usage];
            }
        }
        else
        {
            switch (resource->info.shaderStage)
            {
            case ShaderStage::ALL:
                if (m_pipeline->getBuilder().getType() == PipelineType::GRAPHICS)
                {
                    [m_encoder setVertexBuffer:static_cast<MetalBuffer*>(buffer.get())->get()
                                        offset:0
                                       atIndex:resource->set];
                    [m_encoder setFragmentBuffer:static_cast<MetalBuffer*>(buffer.get())->get()
                                          offset:0
                                         atIndex:resource->set];
                }
                else if (m_pipeline->getBuilder().getType() == PipelineType::COMPUTE)
                {
                    [m_computeEncoder setBuffer:static_cast<MetalBuffer*>(buffer.get())->get()
                                         offset:0
                                        atIndex:resource->set];
                }
                break;
            case ShaderStage::VERTEX:
                [m_encoder setVertexBuffer:static_cast<MetalBuffer*>(buffer.get())->get()
                                    offset:0
                                   atIndex:resource->set];
                break;
            case ShaderStage::FRAGMENT:
                [m_encoder setFragmentBuffer:static_cast<MetalBuffer*>(buffer.get())->get()
                                      offset:0
                                     atIndex:resource->set];
                break;
            case ShaderStage::COMPUTE:
                [m_computeEncoder setBuffer:static_cast<MetalBuffer*>(buffer.get())->get()
                                     offset:0
                                    atIndex:resource->set];
                break;
            default:
                break;
            }
        }
    }
}

void MetalRenderContext::bindTexture(Ref<Texture> texture, std::string_view name)
{
    if (!texture.valid())
    {
        log(LogLevel::WARNING, "Could not bind texture, reference invalid");
        return;
    }

    std::optional<ResourcePosition> resource = m_pipeline->getShaderModule().getResource(name);
    if (!resource.has_value())
    {
        log(LogLevel::WARNING, "Could not bind texture, no resource named \"{}\"", name);
        return;
    }

    if (resource.value().info.type != ResourceType::TEXTURE && resource.value().info.type != ResourceType::RW_TEXTURE)
    {
        log(LogLevel::WARNING, "Could not bind texture, \"{}\" is a {}", name,
            ResourceTypeNames[static_cast<u32>(resource.value().info.type)]);
        return;
    }

    @autoreleasepool
    {
        if (resource->info.partOfArgBuffer)
        {
            m_parameterHandler->writeTexture(*static_cast<MetalTexture*>(texture.get()), resource.value().set,
                                             resource.value().binding);

            MTLResourceUsage usage = MTLResourceUsageRead;
            if (resource.value().info.type != ResourceType::RW_TEXTURE)
            {
                usage |= MTLResourceUsageWrite;
            }

            if (m_pipeline->getBuilder().getType() == PipelineType::GRAPHICS)
            {
                MTLRenderStages stages =
                    resource->info.shaderStage == ShaderStage::FRAGMENT ? MTLRenderStageFragment : MTLRenderStageVertex;
                [m_encoder useResource:static_cast<MetalTexture*>(texture.get())->get() usage:usage stages:stages];
            }
            else if (m_pipeline->getBuilder().getType() == PipelineType::COMPUTE)
            {
                [m_computeEncoder useResource:static_cast<MetalTexture*>(texture.get())->get() usage:usage];
            }
        }
        else
        {
            switch (resource->info.shaderStage)
            {
            case ShaderStage::ALL:
                if (m_pipeline->getBuilder().getType() == PipelineType::GRAPHICS)
                {
                    [m_encoder setVertexTexture:static_cast<MetalTexture*>(texture.get())->get() atIndex:resource->set];
                    [m_encoder setFragmentTexture:static_cast<MetalTexture*>(texture.get())->get()
                                          atIndex:resource->set];
                }
                else if (m_pipeline->getBuilder().getType() == PipelineType::COMPUTE)
                {
                    [m_computeEncoder setTexture:static_cast<MetalTexture*>(texture.get())->get()
                                         atIndex:resource->set];
                }
                break;
            case ShaderStage::VERTEX:
                [m_encoder setVertexTexture:static_cast<MetalTexture*>(texture.get())->get() atIndex:resource->set];
                break;
            case ShaderStage::FRAGMENT:
                [m_encoder setFragmentTexture:static_cast<MetalTexture*>(texture.get())->get() atIndex:resource->set];
                break;
            case ShaderStage::COMPUTE:
                [m_computeEncoder setTexture:static_cast<MetalTexture*>(texture.get())->get() atIndex:resource->set];
                break;
            default:
                break;
            }
        }
    }
}

void MetalRenderContext::bindSampler(const SamplerSettings& sampler, std::string_view name)
{
    std::optional<ResourcePosition> resource = m_pipeline->getShaderModule().getResource(name);
    if (!resource.has_value())
    {
        log(LogLevel::WARNING, "Could not bind sampler, no resource named \"{}\"", name);
        return;
    }

    if (resource.value().info.type != ResourceType::SAMPLER)
    {
        log(LogLevel::WARNING, "Could not bind sampler, \"{}\" is a {}", name,
            ResourceTypeNames[static_cast<u32>(resource.value().info.type)]);
        return;
    }

    @autoreleasepool
    {
        id<MTLSamplerState> metalSampler = m_context->getSampler(sampler);
        if (resource->info.partOfArgBuffer)
        {
            m_parameterHandler->writeSampler(metalSampler, resource.value().set, resource.value().binding);
        }
        else
        {
            switch (resource->info.shaderStage)
            {
            case ShaderStage::ALL:
                if (m_pipeline->getBuilder().getType() == PipelineType::GRAPHICS)
                {
                    [m_encoder setVertexSamplerState:metalSampler atIndex:resource->set];
                    [m_encoder setFragmentSamplerState:metalSampler atIndex:resource->set];
                }
                else if (m_pipeline->getBuilder().getType() == PipelineType::COMPUTE)
                {
                    [m_computeEncoder setSamplerState:metalSampler atIndex:resource->set];
                }
                break;
            case ShaderStage::VERTEX:
                [m_encoder setVertexSamplerState:metalSampler atIndex:resource->set];
                break;
            case ShaderStage::FRAGMENT:
                [m_encoder setFragmentSamplerState:metalSampler atIndex:resource->set];
                break;
            case ShaderStage::COMPUTE:
                [m_computeEncoder setSamplerState:metalSampler atIndex:resource->set];
                break;
            default:
                break;
            }
        }
    }
}

void MetalRenderContext::setParameter(void* data, u32 size, std::string_view name)
{
    std::optional<ParameterBinding> parameter = m_pipeline->getShaderModule().getParameter(name);
    if (!parameter.has_value())
    {
        log(LogLevel::WARNING, "Could not set parameter, no parameter named \"{}\"", name);
        return;
    }

    if (parameter.value().size != size)
    {
        log(LogLevel::WARNING, "Could not set parameter \"{}\", got size {}, expected {}", name, size,
            parameter.value().size);
        return;
    }

    m_parameterHandler->writeParameter(parameter.value(), data, size);
}

void MetalRenderContext::draw(u32 vertexCount, u32 instanceCount, u32 vertexOffset, u32 instanceOffset)
{
    if (m_pipeline->getBuilder().getType() != PipelineType::GRAPHICS)
    {
        log(LogLevel::WARNING, "Could not execute draw command, not using a graphics pipeline");
        return;
    }

    m_parameterHandler->bindBuffers(m_encoder);
    [m_encoder drawPrimitives:MTLPrimitiveTypeTriangle
                  vertexStart:vertexOffset
                  vertexCount:vertexCount
                instanceCount:instanceCount
                 baseInstance:instanceOffset];
    m_parameterHandler->updateIndices();
}

void MetalRenderContext::drawIndexed(u32 indexCount, u32 instanceCount, u32 indexOffset, u32 instanceOffset)
{
    if (m_pipeline->getBuilder().getType() != PipelineType::GRAPHICS)
    {
        log(LogLevel::WARNING, "Could not execute drawIndexed call, not using a graphics pipeline");
        return;
    }

    if (m_boundIndexBuffer == nil)
    {
        log(LogLevel::WARNING, "Could not execute drawIndexed command, no index buffer has been bound");
        return;
    }

    m_parameterHandler->bindBuffers(m_encoder);
    [m_encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                          indexCount:indexCount
                           indexType:MTLIndexTypeUInt32
                         indexBuffer:m_boundIndexBuffer
                   indexBufferOffset:indexOffset
                       instanceCount:instanceCount
                          baseVertex:0
                        baseInstance:instanceOffset];
    m_parameterHandler->updateIndices();
}

void MetalRenderContext::dispatchGroups(u32 groupX, u32 groupY, u32 groupZ)
{
    if (m_pipeline->getBuilder().getType() != PipelineType::COMPUTE)
    {
        log(LogLevel::WARNING, "Could not execute dispatchGroups call, not using a compute pipeline");
        return;
    }

    m_parameterHandler->bindBuffers(m_computeEncoder);
    uvec3 computeThreadsPerGroup = m_pipeline->getShaderModule().getComputeThreadsPerGroup();
    [m_computeEncoder dispatchThreadgroups:MTLSizeMake(groupX, groupY, groupZ)
                     threadsPerThreadgroup:MTLSizeMake(computeThreadsPerGroup.x, computeThreadsPerGroup.y,
                                                       computeThreadsPerGroup.z)];
    m_parameterHandler->updateIndices();
}

void MetalRenderContext::dispatch(u32 x, u32 y, u32 z)
{
    if (m_pipeline->getBuilder().getType() != PipelineType::COMPUTE)
    {
        log(LogLevel::WARNING, "Could not execute dispatch call, not using a compute pipeline");
        return;
    }

    m_parameterHandler->bindBuffers(m_computeEncoder);
    uvec3 computeThreadsPerGroup = m_pipeline->getShaderModule().getComputeThreadsPerGroup();
    [m_computeEncoder dispatchThreadgroups:MTLSizeMake((x + computeThreadsPerGroup.x - 1) / computeThreadsPerGroup.x,
                                                       (y + computeThreadsPerGroup.y - 1) / computeThreadsPerGroup.y,
                                                       (z + computeThreadsPerGroup.z - 1) / computeThreadsPerGroup.z)
                     threadsPerThreadgroup:MTLSizeMake(computeThreadsPerGroup.x, computeThreadsPerGroup.y,
                                                       computeThreadsPerGroup.z)];
    m_parameterHandler->updateIndices();
}

} // namespace huedra