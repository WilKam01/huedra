#include "pipeline_builder.hpp"
#include "core/log.hpp"

namespace huedra {

PipelineBuilder& PipelineBuilder::init(PipelineType type)
{
    m_type = type;
    m_shaderStages.clear();
    m_resources.clear();
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

PipelineBuilder& PipelineBuilder::addPushConstantRange(u32 stage, u32 size)
{
    m_pushConstantShaderStage = static_cast<ShaderStageFlags>(stage);
    m_pushConstantRange = size;
    return *this;
}

PipelineBuilder& PipelineBuilder::addResourceSet()
{
    if (m_resources.empty() || !m_resources.back().empty())
    {
        m_resources.push_back({});
    }
    return *this;
}

PipelineBuilder& PipelineBuilder::addResourceBinding(u32 stage, ResourceType resource)
{
    if (m_resources.empty())
    {
        log(LogLevel::ERR, "Could not add resource binding, no resource sets have been added (fix by calling "
                           "addResourceSet beforehand)");
        return *this;
    }

    m_resources.back().push_back({static_cast<ShaderStageFlags>(stage), resource});
    return *this;
}

} // namespace huedra