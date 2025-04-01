#include "context.hpp"
#include "core/file/utils.hpp"
#include "core/log.hpp"

namespace huedra {

void SlangContext::init()
{
    slang::createGlobalSession(m_globalSession.writeRef());

    slang::SessionDesc sessionDesc{};
    sessionDesc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;

    slang::TargetDesc targetDesc{};
    std::vector<slang::CompilerOptionEntry> optionEntries{};

// TODO: Test VulkanInvertY and see if inversion code in renderer can be omitted
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

    std::vector<slang::IComponentType*> componentTypes(static_cast<u64>(entryPointCount + 1));
    componentTypes[0] = shaderModule;
    for (i32 i = 0; i < entryPointCount; ++i)
    {
        Slang::ComPtr<slang::IEntryPoint> entryPoint;
        shaderModule->getDefinedEntryPoint(i, entryPoint.writeRef());
        log(LogLevel::D_INFO, "createModule(): Entry point name[i] = {}",
            entryPoint->getFunctionReflection()->getName());

        componentTypes[i + 1] = entryPoint;
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

    Slang::ComPtr<slang::IComponentType> linkedProgram;
    result = composedProgram->link(linkedProgram.writeRef(), diagnositicBlob.writeRef());
    if (SLANG_FAILED(result))
    {
        log(LogLevel::WARNING, "createModule(): link error: {}",
            reinterpret_cast<const char*>(diagnositicBlob->getBufferPointer()));
        return {};
    }

    Slang::ComPtr<slang::IBlob> code;
    // result = linkedProgram->getEntryPointCode(0, 0, code.writeRef(), diagnositicBlob.writeRef());
    result = linkedProgram->getTargetCode(0, code.writeRef(), diagnositicBlob.writeRef());
    if (SLANG_FAILED(result))
    {
        log(LogLevel::WARNING, "createModule(): get target point code error: {}",
            reinterpret_cast<const char*>(diagnositicBlob->getBufferPointer()));
        return {};
    }

    ShaderModule shader;
    shader.init(reinterpret_cast<const u8*>(code->getBufferPointer()), code->getBufferSize());
    return shader;
}

} // namespace huedra