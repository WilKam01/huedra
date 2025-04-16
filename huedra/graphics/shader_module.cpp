#include "shader_module.hpp"

#include "core/global.hpp"

#include <ranges>

namespace huedra {

void ShaderModule::init(Slang::ComPtr<slang::IModule> shaderModule,
                        const std::vector<Slang::ComPtr<slang::IEntryPoint>>& entryPoints,
                        const std::vector<SlangStage>& stages)
{
    m_module = shaderModule;
    m_name = shaderModule->getName();
    m_slangEntryPoints = entryPoints;
    m_entryPoints.resize(m_slangEntryPoints.size());

    for (u64 i = 0; i < m_entryPoints.size(); ++i)
    {
        m_entryPoints[i].name = m_slangEntryPoints[i]->getFunctionReflection()->getName();
        m_entryPoints[i].stage = convertShaderStage(stages[i]);
    }
}

ShaderStage ShaderModule::getShaderStage(const std::string& entryPoint) const
{
    ShaderStage stage = ShaderStage::NONE;

    auto it =
        std::ranges::find_if(m_entryPoints, [entryPoint](const EntryPoint& entry) { return entry.name == entryPoint; });
    if (it != m_entryPoints.end())
    {
        stage = it->stage;
    }

    return stage;
}

ShaderStage ShaderModule::convertShaderStage(SlangStage shaderStage)
{
    switch (shaderStage)
    {
    case SLANG_STAGE_VERTEX:
        return ShaderStage::VERTEX;
    case SLANG_STAGE_FRAGMENT:
        return ShaderStage::FRAGMENT;
    case SLANG_STAGE_COMPUTE:
        return ShaderStage::COMPUTE;
    default:
        return ShaderStage::NONE;
    }
    return ShaderStage::NONE;
}

void CompiledShaderModule::init(Slang::ComPtr<slang::IComponentType> program, const u8* code, u64 codeLen)
{
    m_code = std::vector<u8>(code, code + codeLen);
}

} // namespace huedra