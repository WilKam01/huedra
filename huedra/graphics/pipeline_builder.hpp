#pragma once

#include "core/types.hpp"
#include "graphics/pipeline_data.hpp"

namespace huedra {

class PipelineBuilder
{
public:
    PipelineBuilder() = default;
    ~PipelineBuilder() = default;

    PipelineBuilder& init(PipelineType type);

    PipelineBuilder& addShader(ShaderStage stage, std::string shader);

    PipelineBuilder& addVertexInputStream(VertexInputStream inputStream);

    PipelineBuilder& addPushConstantRange(u32 stage, u32 size);
    PipelineBuilder& addResourceSet();
    PipelineBuilder& addResourceBinding(u32 stage, ResourceType resource);

    PipelineType getType() const { return m_type; }
    std::map<ShaderStage, std::string> getShaderStages() const { return m_shaderStages; }
    std::vector<std::vector<ResourceBinding>> getResources() const { return m_resources; }

    std::vector<VertexInputStream> getVertexInputStreams() const { return m_vertexStreams; }

    std::vector<u32> getPushConstantRanges() const { return m_pushConstantRanges; }
    std::vector<ShaderStageFlags> getPushConstantShaderStages() const { return m_pushConstantShaderStages; }

private:
    PipelineType m_type;
    std::map<ShaderStage, std::string> m_shaderStages;
    std::vector<std::vector<ResourceBinding>> m_resources{};

    std::vector<VertexInputStream> m_vertexStreams;

    std::vector<u32> m_pushConstantRanges{};
    std::vector<ShaderStageFlags> m_pushConstantShaderStages{};
};

} // namespace huedra