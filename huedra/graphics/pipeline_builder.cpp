#include "pipeline_builder.hpp"
#include "core/log.hpp"

namespace huedra {

PipelineBuilder& PipelineBuilder::init(PipelineType type)
{
    m_type = type;
    m_shaderStages.clear();
    return *this;
}

PipelineBuilder& PipelineBuilder::addShader(ShaderStage stage, std::string shader)
{
    if (m_shaderStages.contains(stage))
    {
        log(LogLevel::WARNING, "Could not add shader: \"%s\". Shader stage already defined", shader.c_str());
        return *this;
    }

    m_shaderStages.insert(std::make_pair(stage, shader));
    return *this;
}

} // namespace huedra