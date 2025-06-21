#include "context.hpp"
#include "core/file/utils.hpp"
#include "core/log.hpp"
#include "graphics/shader_module.hpp"

namespace huedra {

void SlangContext::init()
{
    slang::createGlobalSession(m_globalSession.writeRef());

    slang::SessionDesc sessionDesc{};
    sessionDesc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;

    slang::TargetDesc targetDesc{};
    std::vector<slang::CompilerOptionEntry> optionEntries{};

#ifdef VULKAN
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = m_globalSession->findProfile("spirv_1_5");

    slang::CompilerOptionEntry capability{.name = slang::CompilerOptionName::Capability};

    capability.value = {.intValue0 = m_globalSession->findCapability("spvImageQuery")};
    optionEntries.push_back(capability);

    capability.value = {.intValue0 = m_globalSession->findCapability("spvSparseResidency")};
    optionEntries.push_back(capability);

    optionEntries.push_back({.name = slang::CompilerOptionName::VulkanUseEntryPointName, .value = {.intValue0 = 1}});
    optionEntries.push_back({.name = slang::CompilerOptionName::VulkanInvertY, .value = {.intValue0 = 1}});
#elif defined(METAL)
    targetDesc.format = SLANG_METAL;
    targetDesc.profile = m_globalSession->findProfile("metal_2_1");
#endif

    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;
    sessionDesc.compilerOptionEntries = optionEntries.data();
    sessionDesc.compilerOptionEntryCount = static_cast<i32>(optionEntries.size());
    sessionDesc.preprocessorMacros = nullptr;
    sessionDesc.preprocessorMacroCount = 0;
    m_globalSession->createSession(sessionDesc, m_session.writeRef());
}

void SlangContext::cleanup() {}

ShaderModule SlangContext::createModule(const std::string& name, const std::string& source)
{
    Slang::ComPtr<slang::IBlob> diagnositicBlob;
    Slang::ComPtr<slang::IModule> shaderModule;
    ShaderModule shader;

    shaderModule = m_session->loadModuleFromSourceString(name.c_str(), (name + ".slang").c_str(), source.c_str(),
                                                         diagnositicBlob.writeRef());
    if (diagnositicBlob != nullptr)
    {
        log(LogLevel::WARNING, "createModule(): load module from string error:\n{}",
            reinterpret_cast<const char*>(diagnositicBlob->getBufferPointer()));
    }
    if (shaderModule == nullptr)
    {
        return {};
    }

    i32 entryPointCount = shaderModule->getDefinedEntryPointCount();
    if (entryPointCount == 0)
    {
        log(LogLevel::WARNING, "createModule(): Could not find any entry points");
        return {};
    }

    std::vector<Slang::ComPtr<slang::IEntryPoint>> entryPoints(static_cast<u64>(entryPointCount));
    std::vector<slang::IComponentType*> componentTypes(static_cast<u64>(entryPointCount) + 1);
    componentTypes[0] = shaderModule;
    for (i32 i = 0; i < entryPointCount; ++i)
    {
        shaderModule->getDefinedEntryPoint(i, entryPoints[i].writeRef());
        componentTypes[i + 1] = entryPoints[i];
    }

    Slang::ComPtr<slang::IComponentType> composedProgram;
    SlangResult result = m_session->createCompositeComponentType(
        componentTypes.data(), componentTypes.size(), composedProgram.writeRef(), diagnositicBlob.writeRef());
    if (SLANG_FAILED(result))
    {
        log(LogLevel::WARNING, "createModule(): create composite component type error: {}",
            reinterpret_cast<const char*>(diagnositicBlob->getBufferPointer()));
        return {};
    }

    std::vector<SlangStage> stages(static_cast<u64>(entryPointCount));
    for (i32 i = 0; i < entryPointCount; ++i)
    {
        stages[i] = composedProgram->getLayout()->getEntryPointByIndex(i)->getStage();
    }

    shader.init(shaderModule, entryPoints, stages);
    return shader;
}

CompiledShaderModule SlangContext::compileAndLinkModules(const std::map<ShaderStage, ShaderInput>& inputs)
{
    Slang::ComPtr<slang::IBlob> diagnositicBlob;
    std::set<ShaderModule*> shaderModules; // Keep track of which module have already been added
    std::vector<slang::IComponentType*> componentTypes;

    for (const auto& [stage, input] : inputs)
    {
        if (!shaderModules.contains(input.shaderModule))
        {
            componentTypes.push_back(input.shaderModule->getSlangModule());
            shaderModules.insert(input.shaderModule);
        }
        componentTypes.push_back(input.shaderModule->getSlangEntryPoint(input.entryPointName));
    }

    Slang::ComPtr<slang::IComponentType> composedProgram;
    SlangResult result = m_session->createCompositeComponentType(
        componentTypes.data(), componentTypes.size(), composedProgram.writeRef(), diagnositicBlob.writeRef());
    if (SLANG_FAILED(result))
    {
        log(LogLevel::WARNING, "compileAndLinkModules(): create composite component type error: {}",
            reinterpret_cast<const char*>(diagnositicBlob->getBufferPointer()));
        return {};
    }

    Slang::ComPtr<slang::IComponentType> linkedProgram;
    result = composedProgram->link(linkedProgram.writeRef(), diagnositicBlob.writeRef());
    if (SLANG_FAILED(result))
    {
        log(LogLevel::WARNING, "compileAndLinkModules(): link error: {}",
            reinterpret_cast<const char*>(diagnositicBlob->getBufferPointer()));
        return {};
    }

    Slang::ComPtr<slang::IBlob> codeBlob;
    result = linkedProgram->getTargetCode(0, codeBlob.writeRef(), diagnositicBlob.writeRef());
    if (SLANG_FAILED(result))
    {
        log(LogLevel::WARNING, "compileAndLinkModules(): get target point code error: {}",
            reinterpret_cast<const char*>(diagnositicBlob->getBufferPointer()));
        return {};
    }

    CompiledShaderModule compiledModule;
    compiledModule.init(linkedProgram, reinterpret_cast<const u8*>(codeBlob->getBufferPointer()),
                        codeBlob->getBufferSize());
    return compiledModule;
}

} // namespace huedra