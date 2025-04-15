#include "shader_module.hpp"

#include "core/global.hpp"

namespace huedra {

void ShaderModule::init(Slang::ComPtr<slang::IModule> shaderModule)
{
    m_module = shaderModule;
    m_name = shaderModule->getName();

    i32 entryPointCount = shaderModule->getDefinedEntryPointCount();
    if (entryPointCount == 0)
    {
        log(LogLevel::WARNING, "ShaderModule::init(): Could not find any entry points");
        return;
    }

    // TODO: Currently reflection not working (maybe compile just to get name and stage?)
    m_entryPoints.resize(static_cast<u32>(entryPointCount));
    m_slangEntryPoints.resize(static_cast<u32>(entryPointCount));
    for (i32 i = 0; i < entryPointCount; ++i)
    {
        Slang::ComPtr<slang::IEntryPoint> entryPoint;
        shaderModule->getDefinedEntryPoint(i, entryPoint.writeRef());
        m_slangEntryPoints[static_cast<u32>(i)] = entryPoint;

        EntryPoint entry{};
        auto funcReflection = entryPoint->getFunctionReflection();
        entry.name = funcReflection->getName();
        entry.stage = ShaderStage::NONE;
        for (i32 j = 0; j < funcReflection->getUserAttributeCount(); ++j)
        {
            auto attribute = funcReflection->getUserAttributeByIndex(static_cast<u32>(j));
            std::string attributeName{attribute->getName()};
            if (attributeName == "shader")
            {
                u64 strSize{0};
                const char* str = attribute->getArgumentValueString(0, &strSize);
                std::string stage{str, strSize};

                if (stage == "vertex")
                {
                    entry.stage = ShaderStage::VERTEX;
                }
                else if (stage == "fragment")
                {
                    entry.stage = ShaderStage::FRAGMENT;
                }
                else if (stage == "compute")
                {
                    entry.stage = ShaderStage::COMPUTE;
                }
                break;
            }
        }
    }
}

void CompiledShaderModule::init(Slang::ComPtr<slang::IComponentType> program, const u8* code, u64 codeLen)
{
    m_code = std::vector<u8>(code, code + codeLen);
}

} // namespace huedra