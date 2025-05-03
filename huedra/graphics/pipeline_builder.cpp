#include "pipeline_builder.hpp"
#include "core/log.hpp"

#include <functional>

namespace huedra {

PipelineBuilder& PipelineBuilder::init(PipelineType type)
{
    m_initialized = true;
    m_type = type;
    m_shaderStages.clear();
    m_vertexStreams.clear();
    return *this;
}

PipelineBuilder& PipelineBuilder::addShader(ShaderModule& shaderModule, const std::string& entryPointName)
{
    ShaderStage stage = shaderModule.getShaderStage(entryPointName);
    if (stage == ShaderStage::NONE)
    {
        log(LogLevel::WARNING, "PipelineBuilder::addShader(): Could not find entryPoint \"{}\" in module: {}",
            entryPointName.c_str(), shaderModule.getName().c_str());
        return *this;
    }

    if (m_type == PipelineType::COMPUTE && stage != ShaderStage::COMPUTE)
    {
        log(LogLevel::WARNING, "PipelineBuilder.addShader(): Could not add non compute shader to compute pipoeline");
        return *this;
    }

    if (m_shaderStages.contains(stage))
    {
        m_shaderStages[stage] = {.shaderModule = &shaderModule, .entryPointName = entryPointName};
        log(LogLevel::WARNING, "¨PipelineBuilder::addShader(): {} shader overwritten",
            ShaderStageNames[static_cast<u64>(stage)]);
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
        log(LogLevel::WARNING, "PipelineBuilder::addVertexInputStream() used on non Graphics pipeline");
        return *this;
    }
    m_vertexStreams.push_back(inputStream);
    return *this;
}

PipelineBuilder& PipelineBuilder::setPrimitive(PrimitiveType type, PrimitiveLayout layout)
{
    if (m_type != PipelineType::GRAPHICS)
    {
        log(LogLevel::WARNING, "PipelineBuilder::setPrimitive() used on non Graphics pipeline");
        return *this;
    }
    m_primitiveType = type;
    m_primitiveLayout = layout;
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
    for (auto& [stage, input] : m_shaderStages)
    {
        combineHash(u64Hash(static_cast<u64>(stage)));
        combineHash(strHash(input.shaderModule->getName()));
        combineHash(strHash(input.entryPointName));
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