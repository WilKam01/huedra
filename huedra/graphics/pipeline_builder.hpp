#pragma once

#include "core/types.hpp"
#include "graphics/pipeline_data.hpp"
#include "graphics/render_context.hpp"

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

    PipelineBuilder& addShader(ShaderStage stage, ShaderModule& shaderModule, const std::string& entryPointName);

    PipelineBuilder& addVertexInputStream(const VertexInputStream& inputStream);

    PipelineBuilder& addPushConstantRange(u32 stage, u32 size);
    PipelineBuilder& addResourceSet();
    PipelineBuilder& addResourceBinding(u32 stage, ResourceType resource);

    u64 generateHash();

    bool empty() const { return !m_initialized; }
    u64 getHash() const { return m_hash; }

    PipelineType getType() const { return m_type; }
    std::map<ShaderStage, ShaderInput> getShaderStages() const { return m_shaderStages; }
    std::vector<std::vector<ResourceBinding>> getResources() const { return m_resources; }

    std::vector<VertexInputStream> getVertexInputStreams() const { return m_vertexStreams; }

    std::vector<u32> getPushConstantRanges() const { return m_pushConstantRanges; }
    std::vector<ShaderStageFlags> getPushConstantShaderStages() const { return m_pushConstantShaderStages; }

private:
    bool m_initialized{false};
    u64 m_hash{0};

    PipelineType m_type{PipelineType::GRAPHICS};
    std::map<ShaderStage, ShaderInput> m_shaderStages;
    std::vector<std::vector<ResourceBinding>> m_resources;

    std::vector<u32> m_pushConstantRanges;
    std::vector<ShaderStageFlags> m_pushConstantShaderStages;

    // Graphics specific
    std::vector<VertexInputStream> m_vertexStreams;
};

} // namespace huedra