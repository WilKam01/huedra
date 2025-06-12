#include "parameterHandler.hpp"
#include "core/log.hpp"
#include "graphics/buffer.hpp"
#include "graphics/pipeline_data.hpp"
#include "graphics/shader_module.hpp"
#include "platform/metal/pipeline.hpp"
#include <Metal/Metal.h>
#include <cstring>

namespace huedra {

void MetalParameterHandler::init(id<MTLDevice> device, const MetalPipeline& pipeline,
                                 CompiledShaderModule& shaderModule)
{
    m_device = device;
    const std::vector<std::vector<ResourceBinding>>& buffers = shaderModule.getMetalBuffers();
    for (u32 i = 0; i < buffers.size(); ++i)
    {
        if (buffers[i][0].partOfArgBuffer)
        {
            ArgBufferInfo argBuffer;
            argBuffer.shaderStage = buffers[i][0].shaderStage;
            for (const auto& resource : buffers[i])
            {
                argBuffer.types.push_back(resource.type);
            }

            if (pipeline.getBuilder().getType() == PipelineType::GRAPHICS)
            {
                // Use vertex if all is used, should be the same layout
                if (buffers[i][0].shaderStage == ShaderStage::VERTEX || buffers[i][0].shaderStage == ShaderStage::ALL)
                {
                    argBuffer.argEncoder =
                        [pipeline.getFunction(ShaderStage::VERTEX).value() newArgumentEncoderWithBufferIndex:i];
                }
                else if (buffers[i][0].shaderStage == ShaderStage::FRAGMENT)
                {
                    argBuffer.argEncoder =
                        [pipeline.getFunction(ShaderStage::FRAGMENT).value() newArgumentEncoderWithBufferIndex:i];
                }
            }
            else if (pipeline.getBuilder().getType() == PipelineType::COMPUTE)
            {
                // Same outcome regardless of ShaderStage::ALL or ShaderStage::COMPUTE
                argBuffer.argEncoder =
                    [pipeline.getFunction(ShaderStage::COMPUTE).value() newArgumentEncoderWithBufferIndex:i];
            }
            m_argBuffers.insert(std::pair<u32, ArgBufferInfo>(i, argBuffer));
        }
    }

    const std::map<ShaderStage, CompiledShaderModule::MetalShaderStageParametersInfo> metalParameters =
        pipeline.getShaderModule().getMetalParameters();

    for (const auto& [stage, param] : metalParameters)
    {
        ParameterInfo parameter;
        parameter.bufferIndex = param.bufferIndex;
        parameter.startByte = param.startByte;
        m_parameters.insert(std::pair<ShaderStage, ParameterInfo>(stage, parameter));
    }
}

void MetalParameterHandler::cleanup()
{
    for (auto& [set, argBuffer] : m_argBuffers)
    {
        [argBuffer.argEncoder release];
        for (auto& buffer : argBuffer.buffers)
        {
            [buffer release];
        }
        argBuffer.curIndex = 0;
        argBuffer.shaderStage = ShaderStage::NONE;
        argBuffer.updatedSinceLastUpdate = false;
        argBuffer.types.clear();
    }

    for (auto& [stage, info] : m_parameters)
    {
        for (auto& buffer : info.buffers)
        {
            [buffer release];
        }
        info.curBufferIndex = 0;
        info.curByteIndex = 0;
        info.updatedSinceLastUpdate = false;
    }
}

void MetalParameterHandler::writeBuffer(MetalBuffer& buffer, u32 set, u32 binding)
{
    if (!m_argBuffers.contains(set))
    {
        log(LogLevel::WARNING,
            "MetalParameterHandler::writeBuffer(): Could not set buffer to argument buffer, set {} is not an argument "
            "buffer",
            set);
        return;
    }

    if (binding >= m_argBuffers[set].types.size() ||
        (m_argBuffers[set].types[binding] != ResourceType::CONSTANT_BUFFER &&
         m_argBuffers[set].types[binding] != ResourceType::STRUCTURED_BUFFER))
    {
        log(LogLevel::WARNING,
            "MetalParameterHandler::writeBuffer(): Could not set buffer to argument buffer, binding {} is not "
            "valid",
            binding);
        return;
    }

    @autoreleasepool
    {
        // Need to create new argument buffer
        if (m_argBuffers[set].curIndex == m_argBuffers[set].buffers.size())
        {
            id<MTLBuffer> argumentBuffer = [m_device newBufferWithLength:m_argBuffers[set].argEncoder.encodedLength
                                                                 options:MTLResourceStorageModeShared];
            m_argBuffers[set].buffers.push_back(argumentBuffer);
        }

        [m_argBuffers[set].argEncoder setArgumentBuffer:m_argBuffers[set].buffers[m_argBuffers[set].curIndex] offset:0];
        [m_argBuffers[set].argEncoder setBuffer:buffer.get() offset:0 atIndex:binding];
        m_argBuffers[set].updatedSinceLastUpdate = true;
    }
}

void MetalParameterHandler::writeTexture(MetalTexture& texture, u32 set, u32 binding)
{
    if (!m_argBuffers.contains(set))
    {
        log(LogLevel::WARNING,
            "MetalParameterHandler::writeTexture(): Could not set texture to argument buffer, set {} is not an "
            "argument buffer",
            set);
        return;
    }

    if (binding >= m_argBuffers[set].types.size() || (m_argBuffers[set].types[binding] != ResourceType::TEXTURE &&
                                                      m_argBuffers[set].types[binding] != ResourceType::RW_TEXTURE))
    {
        log(LogLevel::WARNING,
            "MetalParameterHandler::writeTexture(): Could not set texture to argument buffer, binding {} is not valid",
            binding);
        return;
    }

    @autoreleasepool
    {
        // Need to create new argument buffer
        if (m_argBuffers[set].curIndex == m_argBuffers[set].buffers.size())
        {
            id<MTLBuffer> argumentBuffer = [m_device newBufferWithLength:m_argBuffers[set].argEncoder.encodedLength
                                                                 options:MTLResourceStorageModeShared];
            m_argBuffers[set].buffers.push_back(argumentBuffer);
        }

        [m_argBuffers[set].argEncoder setArgumentBuffer:m_argBuffers[set].buffers[m_argBuffers[set].curIndex] offset:0];
        [m_argBuffers[set].argEncoder setTexture:texture.get() atIndex:binding];
        m_argBuffers[set].updatedSinceLastUpdate = true;
    }
}

void MetalParameterHandler::writeSampler(id<MTLSamplerState> sampler, u32 set, u32 binding)
{
    if (!m_argBuffers.contains(set))
    {
        log(LogLevel::WARNING,
            "MetalParameterHandler::writeSampler(): Could not set sampler to argument buffer, set {} is not an "
            "argument buffer",
            set);
        return;
    }

    if (binding >= m_argBuffers[set].types.size() || m_argBuffers[set].types[binding] != ResourceType::SAMPLER)
    {
        log(LogLevel::WARNING,
            "MetalParameterHandler::writeSampler(): Could not set sampler to argument buffer, binding {} is not valid",
            binding);
        return;
    }

    @autoreleasepool
    {
        // Need to create new argument buffer
        if (m_argBuffers[set].curIndex == m_argBuffers[set].buffers.size())
        {
            id<MTLBuffer> argumentBuffer = [m_device newBufferWithLength:m_argBuffers[set].argEncoder.encodedLength
                                                                 options:MTLResourceStorageModeShared];
            m_argBuffers[set].buffers.push_back(argumentBuffer);
        }

        [m_argBuffers[set].argEncoder setArgumentBuffer:m_argBuffers[set].buffers[m_argBuffers[set].curIndex] offset:0];
        [m_argBuffers[set].argEncoder setSamplerState:sampler atIndex:binding];
        m_argBuffers[set].updatedSinceLastUpdate = true;
    }
}

void MetalParameterHandler::writeParameter(const ParameterBinding& parameter, void* data, u32 size)
{
    @autoreleasepool
    {
        ParameterInfo& info = m_parameters[parameter.shaderStage];
        if (info.curBufferIndex == info.buffers.size())
        {
            id<MTLBuffer> buffer = [m_device newBufferWithLength:PARAMETER_BUFFER_SIZE
                                                         options:MTLResourceStorageModeShared];
            info.buffers.push_back(buffer);
        }

        void* dst = reinterpret_cast<u8*>([info.buffers[info.curBufferIndex] contents]) + info.curByteIndex +
                    (parameter.offset - info.startByte);
        std::memcpy(dst, data, size);
        info.updatedSinceLastUpdate = true;
    }
}

void MetalParameterHandler::bindBuffers(id<MTLRenderCommandEncoder> encoder)
{
    // Argument buffers
    for (auto& [set, argBuffer] : m_argBuffers)
    {
        if (argBuffer.updatedSinceLastUpdate)
        {
            switch (argBuffer.shaderStage)
            {
            case ShaderStage::ALL:
                [encoder setVertexBuffer:argBuffer.buffers[argBuffer.curIndex] offset:0 atIndex:set];
                [encoder setFragmentBuffer:argBuffer.buffers[argBuffer.curIndex] offset:0 atIndex:set];
                break;
            case ShaderStage::VERTEX:
                [encoder setVertexBuffer:argBuffer.buffers[argBuffer.curIndex] offset:0 atIndex:set];
                break;
            case ShaderStage::FRAGMENT:
                [encoder setFragmentBuffer:argBuffer.buffers[argBuffer.curIndex] offset:0 atIndex:set];
                break;
            default:
                break;
            }
        }
    }

    // Parameter buffers
    for (auto& [stage, info] : m_parameters)
    {
        if (info.updatedSinceLastUpdate)
        {
            switch (stage)
            {
            case ShaderStage::ALL:
                [encoder setVertexBuffer:info.buffers[info.curBufferIndex]
                                  offset:info.curByteIndex
                                 atIndex:info.bufferIndex];
                [encoder setFragmentBuffer:info.buffers[info.curBufferIndex]
                                    offset:info.curByteIndex
                                   atIndex:info.bufferIndex];
                break;
            case ShaderStage::VERTEX:
                [encoder setVertexBuffer:info.buffers[info.curBufferIndex]
                                  offset:info.curByteIndex
                                 atIndex:info.bufferIndex];
                break;
            case ShaderStage::FRAGMENT:
                [encoder setFragmentBuffer:info.buffers[info.curBufferIndex]
                                    offset:info.curByteIndex
                                   atIndex:info.bufferIndex];
                break;
            default:
                break;
            }
        }
    }
}

void MetalParameterHandler::bindBuffers(id<MTLComputeCommandEncoder> encoder)
{
    // Argument buffers
    for (auto& [set, argBuffer] : m_argBuffers)
    {
        if (argBuffer.updatedSinceLastUpdate)
        {
            switch (argBuffer.shaderStage)
            {
            case ShaderStage::ALL:
            case ShaderStage::COMPUTE:
                [encoder setBuffer:argBuffer.buffers[argBuffer.curIndex] offset:0 atIndex:set];
                break;
            default:
                break;
            }
        }
    }

    // Parameter buffers
    for (auto& [stage, info] : m_parameters)
    {
        if (info.updatedSinceLastUpdate)
        {
            switch (stage)
            {
            case ShaderStage::ALL:
            case ShaderStage::COMPUTE:
                [encoder setBuffer:info.buffers[info.curBufferIndex] offset:info.curByteIndex atIndex:info.bufferIndex];
                break;
            default:
                break;
            }
        }
    }
}

void MetalParameterHandler::resetIndices()
{
    for (auto& [set, argBuffer] : m_argBuffers)
    {
        argBuffer.curIndex = 0;
        argBuffer.updatedSinceLastUpdate = false;
    }

    for (auto& [stage, info] : m_parameters)
    {
        info.curBufferIndex = 0;
        info.curByteIndex = 0;
        info.updatedSinceLastUpdate = false;
    }
}

void MetalParameterHandler::updateIndices()
{
    for (auto& [set, argBuffer] : m_argBuffers)
    {
        if (argBuffer.updatedSinceLastUpdate)
        {
            ++argBuffer.curIndex;
            argBuffer.updatedSinceLastUpdate = false;
        }
    }

    for (auto& [stage, info] : m_parameters)
    {
        if (info.updatedSinceLastUpdate)
        {
            // Times 2 since size of the range is also the aligned offset
            if (info.curByteIndex + PARAMETER_ALIGNED_OFFSET * 2 > PARAMETER_BUFFER_SIZE)
            {
                ++info.curBufferIndex;
                info.curByteIndex = 0;
            }
            else
            {
                info.curByteIndex += PARAMETER_ALIGNED_OFFSET;
            }
            info.updatedSinceLastUpdate = false;
        }
    }
}

} // namespace huedra