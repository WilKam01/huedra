#pragma once

#include "core/types.hpp"
#include "graphics/pipeline_data.hpp"
#include "graphics/render_context.hpp"
#include "graphics/shader_module.hpp"

namespace huedra {

class PipelineBuilder
{
public:
    PipelineBuilder() = default;
    ~PipelineBuilder() = default;

    PipelineBuilder(const PipelineBuilder& rhs) = default;
    PipelineBuilder& operator=(const PipelineBuilder& rhs) = default;
    PipelineBuilder(PipelineBuilder&& rhs) = default;
    PipelineBuilder& operator=(PipelineBuilder&& rhs) = default;

    PipelineBuilder& init(PipelineType type);
    PipelineBuilder& addShader(ShaderModule& shaderModule, const std::string& entryPointName);
    PipelineBuilder& addVertexInputStream(const VertexInputStream& inputStream);

    u64 generateHash();

    bool empty() const { return !m_initialized; }
    u64 getHash() const { return m_hash; }

    PipelineType getType() const { return m_type; }
    std::map<ShaderStage, ShaderInput> getShaderStages() const { return m_shaderStages; }
    std::vector<VertexInputStream> getVertexInputStreams() const { return m_vertexStreams; }

private:
    bool m_initialized{false};
    u64 m_hash{0};

    PipelineType m_type{PipelineType::GRAPHICS};
    std::map<ShaderStage, ShaderInput> m_shaderStages;

    // Graphics specific
    std::vector<VertexInputStream> m_vertexStreams;
};

} // namespace huedra