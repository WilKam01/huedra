#pragma once

#include "graphics/pipeline_builder.hpp"
#include "graphics/shader_module.hpp"
#include "platform/metal/config.hpp"

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

private:
    id<MTLDevice> m_device;
    id<MTLRenderPipelineState> m_pipeline;
    CompiledShaderModule m_shaderModule;
    PipelineBuilder m_builder;
};

} // namespace huedra