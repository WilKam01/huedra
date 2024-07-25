#pragma once

#include "core/types.hpp"

namespace huedra {

enum class ShaderStage
{
    VERTEX,
    FRAGMENT
};

enum class PipelineType
{
    GRAPHICS
};

class PipelineBuilder
{
public:
    PipelineBuilder() = default;
    ~PipelineBuilder() = default;

    PipelineBuilder& init(PipelineType type);

    PipelineBuilder& addShader(ShaderStage stage, std::string shader);

    PipelineType getType() const { return m_type; }
    std::map<ShaderStage, std::string> getShaderStages() const { return m_shaderStages; }

private:
    PipelineType m_type;
    std::map<ShaderStage, std::string> m_shaderStages;
};

} // namespace huedra