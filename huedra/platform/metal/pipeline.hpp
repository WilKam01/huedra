#pragma once

#include "graphics/pipeline_builder.hpp"
#include "graphics/pipeline_data.hpp"
#include "graphics/render_pass_builder.hpp"
#include "graphics/shader_module.hpp"
#include "platform/metal/config.hpp"
#include "platform/metal/parameter_handler.hpp"

namespace huedra {

class MetalPipeline
{
public:
    MetalPipeline() = default;
    virtual ~MetalPipeline() = default;

    MetalPipeline(const MetalPipeline& rhs) = default;
    MetalPipeline& operator=(const MetalPipeline& rhs) = default;
    MetalPipeline(MetalPipeline&& rhs) = default;
    MetalPipeline& operator=(MetalPipeline&& rhs) = default;

    void initGraphics(id<MTLDevice> device, const PipelineBuilder& pipelineBuilder,
                      std::vector<RenderTargetInfo> renderTargets);
    void initCompute(id<MTLDevice> device, const PipelineBuilder& pipelineBuilder);
    void cleanup();

    id<MTLRenderPipelineState> getGraphicsPipeline() const { return m_renderPipeline; }
    id<MTLComputePipelineState> getComputePipeline() const { return m_computePipeline; }
    const PipelineBuilder& getBuilder() const { return m_builder; }
    const CompiledShaderModule& getShaderModule() const { return m_shaderModule; }
    std::optional<id<MTLFunction>> getFunction(ShaderStage stage) const;
    MetalParameterHandler& getParameterHandler();

private:
    id<MTLDevice> m_device;
    id<MTLRenderPipelineState> m_renderPipeline;
    id<MTLComputePipelineState> m_computePipeline;
    std::map<ShaderStage, id<MTLFunction>> m_functions;
    PipelineBuilder m_builder;
    CompiledShaderModule m_shaderModule;
    std::vector<MetalParameterHandler> m_parameterHandlers;
};

} // namespace huedra