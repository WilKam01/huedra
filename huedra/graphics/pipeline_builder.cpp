#include "pipeline_builder.hpp"
#include "core/log.hpp"

namespace huedra {

PipelineBuilder& PipelineBuilder::init(PipelineType type)
{
    m_type = type;
    m_shaderStages.clear();
    m_resources.clear();
    m_vertexStreams.clear();
    m_pushConstantRanges.clear();
    m_pushConstantShaderStages.clear();
    m_renderCommands = nullptr;
    return *this;
}

PipelineBuilder& PipelineBuilder::addShader(ShaderStage stage, std::string shader)
{
    if (m_shaderStages.contains(stage))
    {
        m_shaderStages[stage] = shader;
        return *this;
    }

    m_shaderStages.insert(std::make_pair(stage, shader));
    return *this;
}

PipelineBuilder& PipelineBuilder::addVertexInputStream(VertexInputStream inputStream)
{
    if (m_type != PipelineType::GRAPHICS)
    {
        log(LogLevel::WARNING, "addVertexInputStream() used on non Graphics pipeline");
        return *this;
    }
    m_vertexStreams.push_back(inputStream);
    return *this;
}

PipelineBuilder& PipelineBuilder::addPushConstantRange(u32 stage, u32 size)
{
    for (auto& shaderStage : m_pushConstantShaderStages)
    {
        if (stage & shaderStage)
        {
            log(LogLevel::ERR, "Could not add push constant range, shader stage was previously defined");
        }
    }

    m_pushConstantShaderStages.push_back(static_cast<ShaderStageFlags>(stage));
    m_pushConstantRanges.push_back(size);
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

PipelineBuilder& PipelineBuilder::setRenderCommands(std::function<void(RenderContext&)> lambda)
{
    m_renderCommands = lambda;
    return *this;
}

} // namespace huedra