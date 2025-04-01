#include "pipeline_builder.hpp"
#include "core/log.hpp"

#include <functional>

namespace huedra {

PipelineBuilder& PipelineBuilder::init(PipelineType type)
{
    m_initialized = true;
    m_type = type;
    m_shaderStages.clear();
    m_resources.clear();
    m_vertexStreams.clear();
    m_parameterRanges.clear();
    m_parameterShaderStages.clear();
    return *this;
}

PipelineBuilder& PipelineBuilder::addShader(ShaderStage stage, ShaderModule& shaderModule,
                                            const std::string& entryPointName)
{
    if (m_type == PipelineType::COMPUTE && stage != ShaderStage::COMPUTE)
    {
        log(LogLevel::WARNING, "Could not add non compute shader to compute pipoeline");
        return *this;
    }

    if (m_shaderStages.contains(stage))
    {
        m_shaderStages[stage] = {.shaderModule = &shaderModule, .entryPointName = entryPointName};
        return *this;
    }

    m_shaderStages.insert(
        std::pair<ShaderStage, ShaderInput>(stage, {.shaderModule = &shaderModule, .entryPointName = entryPointName}));
    return *this;
}

PipelineBuilder& PipelineBuilder::addVertexInputStream(const VertexInputStream& inputStream)
{
    if (m_type != PipelineType::GRAPHICS)
    {
        log(LogLevel::WARNING, "addVertexInputStream() used on non Graphics pipeline");
        return *this;
    }
    m_vertexStreams.push_back(inputStream);
    return *this;
}

PipelineBuilder& PipelineBuilder::addParameterRange(u32 stage, u32 size)
{
    for (auto& shaderStage : m_parameterShaderStages)
    {
        if ((stage & shaderStage) != 0u)
        {
            log(LogLevel::WARNING, "Could not add push constant range, shader stage was previously defined");
            return *this;
        }
    }

    m_parameterShaderStages.push_back(static_cast<ShaderStageFlags>(stage));
    m_parameterRanges.push_back(size);
    return *this;
}

PipelineBuilder& PipelineBuilder::addResourceSet()
{
    if (m_resources.empty() || !m_resources.back().empty())
    {
        m_resources.emplace_back();
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

    m_resources.back().push_back({.shaderStage = static_cast<ShaderStageFlags>(stage), .resource = resource});
    return *this;
}

u64 PipelineBuilder::generateHash()
{
    u64 fnvPrime = 0x00000100000001b3;
    m_hash = 0xcbf29ce484222325;
    auto combineHash = [this, fnvPrime](u64 val) { m_hash ^= val * fnvPrime; };
    auto u64Hash = std::hash<u64>();
    auto u32Hash = std::hash<u32>();
    auto strHash = std::hash<std::string>();

    combineHash(u64Hash(static_cast<u64>(m_type)));
    combineHash(u64Hash(m_shaderStages.size()));
    for (auto& [stage, path] : m_shaderStages)
    {
        combineHash(u64Hash(static_cast<u64>(stage)));
        // TODO: Fix when adding multiple entry points
        // combineHash(strHash(path));
    }

    combineHash(u64Hash(m_resources.size()));
    for (auto& set : m_resources)
    {
        combineHash(u64Hash(set.size()));
        for (auto& binding : set)
        {
            combineHash(u64Hash(static_cast<u64>(binding.shaderStage)));
            combineHash(u64Hash(static_cast<u64>(binding.resource)));
        }
    }

    combineHash(u64Hash(m_parameterRanges.size()));
    for (u64 i = 0; i < m_parameterRanges.size(); ++i)
    {
        combineHash(u32Hash(m_parameterRanges[i]));
        combineHash(u64Hash(static_cast<u64>(m_parameterShaderStages[i])));
    }

    combineHash(u64Hash(m_vertexStreams.size()));
    for (auto& stream : m_vertexStreams)
    {
        combineHash(u64Hash(stream.size));
        combineHash(u64Hash(static_cast<u64>(stream.inputRate)));
        combineHash(u64Hash(stream.attributes.size()));
        for (auto& attribute : stream.attributes)
        {
            combineHash(u64Hash(static_cast<u64>(attribute.format)));
            combineHash(u32Hash(attribute.offset));
        }
    }

    return m_hash;
}

} // namespace huedra