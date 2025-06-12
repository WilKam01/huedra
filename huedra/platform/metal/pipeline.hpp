#pragma once

#include "graphics/pipeline_builder.hpp"
#include "graphics/pipeline_data.hpp"
#include "graphics/shader_module.hpp"
#include "platform/metal/config.hpp"
#include "platform/metal/parameterHandler.hpp"

namespace huedra {

class MetalPipeline
{
public:
    MetalPipeline() = default;
    virtual ~MetalPipeline() = default;

    MetalPipeline(const MetalPipeline& rhs) = delete;
    MetalPipeline& operator=(const MetalPipeline& rhs) = delete;
    MetalPipeline(MetalPipeline&& rhs) = delete;
    MetalPipeline& operator=(MetalPipeline&& rhs) = delete;

    void initGraphics(const PipelineBuilder& pipelineBuilder, id<MTLDevice> device);
    void cleanup();

    id<MTLRenderPipelineState> get() const { return m_pipeline; }
    const PipelineBuilder& getBuilder() const { return m_builder; }
    const CompiledShaderModule& getShaderModule() const { return m_shaderModule; }
    std::optional<id<MTLFunction>> getFunction(ShaderStage stage) const;
    MetalParameterHandler& getParameterHandler();

private:
    id<MTLDevice> m_device;
    id<MTLRenderPipelineState> m_pipeline;
    std::map<ShaderStage, id<MTLFunction>> m_functions;
    PipelineBuilder m_builder;
    CompiledShaderModule m_shaderModule;
    std::vector<MetalParameterHandler> m_parameterHandlers;
};

} // namespace huedra