#pragma once

#include "core/types.hpp"
#include "graphics/pipeline_data.hpp"
#include "graphics/shader_module.hpp"
#include "platform/metal/buffer.hpp"
#include "platform/metal/config.hpp"
#include "platform/metal/texture.hpp"

namespace huedra {

class MetalPipeline;

class MetalParameterHandler
{
public:
    MetalParameterHandler() = default;
    virtual ~MetalParameterHandler() = default;

    MetalParameterHandler(const MetalParameterHandler& rhs) = default;
    MetalParameterHandler& operator=(const MetalParameterHandler& rhs) = default;
    MetalParameterHandler(MetalParameterHandler&& rhs) = default;
    MetalParameterHandler& operator=(MetalParameterHandler&& rhs) = default;

    void init(id<MTLDevice> device, const MetalPipeline& pipeline, CompiledShaderModule& shaderModule);
    void cleanup();

    void writeBuffer(MetalBuffer& buffer, u32 set, u32 binding);
    void writeTexture(MetalTexture& texture, u32 set, u32 binding);
    void writeSampler(id<MTLSamplerState> sampler, u32 set, u32 binding);
    void writeParameter(const ParameterBinding& parameter, void* data, u32 size);

    void bindBuffers(id<MTLRenderCommandEncoder> encoder);
    void bindBuffers(id<MTLComputeCommandEncoder> encoder);

    void resetIndices();
    void updateIndices();

private:
    id<MTLDevice> m_device;

    struct ArgBufferInfo
    {
        u32 curIndex{0};
        bool updatedSinceLastUpdate{false};
        ShaderStage shaderStage{ShaderStage::NONE};
        std::vector<ResourceType> types;
        std::vector<id<MTLBuffer>> buffers;
        id<MTLArgumentEncoder> argEncoder;
    };
    std::map<u32, ArgBufferInfo> m_argBuffers; // Set as key

    struct ParameterInfo
    {
        u32 bufferIndex{0};
        u32 startByte{0};

        u32 curBufferIndex{0};
        u32 curByteIndex{0};
        std::vector<id<MTLBuffer>> buffers;
        bool updatedSinceLastUpdate{false};
    };
    std::map<ShaderStage, ParameterInfo> m_parameters; // ShaderStage as key

    static constexpr u32 PARAMETER_BUFFER_SIZE{1024};
    static constexpr u32 PARAMETER_ALIGNED_OFFSET{256};
};

} // namespace huedra