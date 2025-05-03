#include "shader_module.hpp"

#include "core/file/utils.hpp"
#include "core/global.hpp"
#include "core/string/utils.hpp"

#include <ranges>

namespace huedra {

namespace {

ShaderStage convertSlangShaderStage(SlangStage shaderStage)
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

ResourceType convertSlangResourceType(slang::BindingType bindingType)
{
    switch (bindingType)
    {
    case slang::BindingType::ConstantBuffer:
        return ResourceType::CONSTANT_BUFFER;

    case slang::BindingType::RawBuffer:
    case slang::BindingType::MutableRawBuffer:
        return ResourceType::STRUCTURED_BUFFER;

    case slang::BindingType::Texture:
        return ResourceType::TEXTURE;

    case slang::BindingType::MutableTexture:
        return ResourceType::RW_TEXTURE;

    case slang::BindingType::Sampler:
        return ResourceType::SAMPLER;

    default:
        return ResourceType::NONE;
    }
    return ResourceType::NONE;
}

} // namespace

void ShaderModule::init(Slang::ComPtr<slang::IModule> shaderModule,
                        const std::vector<Slang::ComPtr<slang::IEntryPoint>>& entryPoints,
                        const std::vector<SlangStage>& stages)
{
    m_module = shaderModule;
    m_fullName = shaderModule->getName();
    m_name = splitLastByChars(m_fullName, "/\\")[1];
    m_slangEntryPoints = entryPoints;
    m_entryPoints.resize(m_slangEntryPoints.size());

    log(LogLevel::D_INFO, "Full name {}", m_fullName.c_str());

    for (u64 i = 0; i < m_entryPoints.size(); ++i)
    {
        m_entryPoints[i].name = m_slangEntryPoints[i]->getFunctionReflection()->getName();
        m_entryPoints[i].stage = convertSlangShaderStage(stages[i]);
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

// TODO: add support for resource arrays
void CompiledShaderModule::init(Slang::ComPtr<slang::IComponentType> program, const u8* code, u64 codeLen)
{
    m_code = std::vector<u8>(code, code + codeLen);
    slang::ProgramLayout* programLayout = program->getLayout();
    m_resources.emplace_back(); // First set

    std::vector<std::string_view> descriptorNames;
    std::vector<std::string_view> subObjectNames;

    i32 parameterCount = programLayout->getParameterCount();
    for (i32 i = 0; i < parameterCount; ++i)
    {
        findParameterNames(programLayout->getParameterByIndex(i), descriptorNames, subObjectNames);
    }

    addParameterBlockRanges(ShaderStage::ALL, programLayout->getGlobalParamsTypeLayout(), "", descriptorNames,
                            subObjectNames, 0);

    i32 entryPointCount = programLayout->getEntryPointCount();
    for (i32 i = 0; i < entryPointCount; ++i)
    {
        descriptorNames.clear();
        subObjectNames.clear();

        slang::EntryPointReflection* entryPointLayout = programLayout->getEntryPointByIndex(i);
        parameterCount = entryPointLayout->getParameterCount();
        for (i32 i = 0; i < parameterCount; ++i)
        {
            findParameterNames(entryPointLayout->getParameterByIndex(i), descriptorNames, subObjectNames, true);
        }

        addParameterBlockRanges(convertSlangShaderStage(entryPointLayout->getStage()),
                                entryPointLayout->getTypeLayout(), "", descriptorNames, subObjectNames, 0);
    }
}

std::optional<ResourcePosition> CompiledShaderModule::getResource(std::string_view name) const
{
    for (u32 i = 0; i < m_resources.size(); ++i)
    {
        for (u32 j = 0; j < m_resources[i].size(); ++j)
        {
            if (m_resources[i][j].name == name)
            {
                ResourcePosition resource;
                resource.info = m_resources[i][j];
                resource.set = i;
                resource.binding = j;
                return resource;
            }
        }
    }

    return std::nullopt;
}

std::optional<ParameterBinding> CompiledShaderModule::getParameter(std::string_view name) const
{
    for (u32 i = 0; i < m_parameters.size(); ++i)
    {
        if (m_parameters[i].name == name)
        {
            return m_parameters[i];
        }
    }

    return std::nullopt;
}

JsonObject CompiledShaderModule::getJson() const
{
    JsonObject root;
    for (u32 i = 0; i < m_resources.size(); ++i)
    {
        for (u32 j = 0; j < m_resources[i].size(); ++j)
        {
            std::vector<std::string> splitString = splitByChar(m_resources[i][j].name, '.');
            JsonValue* dest = &root["resources"];
            for (auto& str : splitString)
            {
                dest = &(*dest)[str];
            }

            (*dest)["stage"] = ShaderStageNames[static_cast<u32>(m_resources[i][j].shaderStage)];
            (*dest)["type"] = ResourceTypeNames[static_cast<u32>(m_resources[i][j].type)];
            (*dest)["set"] = i;
            (*dest)["binding"] = j;
        }
    }

    for (auto& param : m_parameters)
    {
        root["push parameters"][param.name]["stage"] = ShaderStageNames[static_cast<u32>(param.shaderStage)];
        root["push parameters"][param.name]["offset"] = param.offset;
        root["push parameters"][param.name]["size"] = param.size;
    }

    return root;
}

void CompiledShaderModule::addParameterBlockRanges(ShaderStage stage, slang::TypeLayoutReflection* typeLayout,
                                                   const std::string& namePrefix,
                                                   const std::vector<std::string_view>& descriptorNames,
                                                   const std::vector<std::string_view>& subObjectNames, u32 setIndex)
{
    if (typeLayout->getSize() > 0)
    {
        ResourceBinding resource;
        resource.name = namePrefix + "<misc>";
        resource.shaderStage = stage;
        resource.type = ResourceType::CONSTANT_BUFFER;
        m_resources[setIndex].push_back(resource);
    }

    i32 rangeCount = typeLayout->getDescriptorSetDescriptorRangeCount(0);
    u32 nameIndex = 0;
    for (i32 i = 0; i < rangeCount; ++i)
    {
        slang::BindingType bindingType = typeLayout->getDescriptorSetDescriptorRangeType(0, i);

        // Skip push constants
        if (bindingType == slang::BindingType::PushConstant)
        {
            continue;
        }

        ResourceBinding resource;
        resource.name = namePrefix + std::string(descriptorNames[nameIndex++]);
        resource.shaderStage = stage;
        resource.type = convertSlangResourceType(bindingType);
        m_resources[setIndex].push_back(resource);
    }

    rangeCount = typeLayout->getSubObjectRangeCount();
    nameIndex = 0;
    for (i32 i = 0; i < rangeCount; ++i)
    {
        i32 bindingRangeIndex = typeLayout->getSubObjectRangeBindingRangeIndex(i);
        slang::BindingType bindingType = typeLayout->getBindingRangeType(bindingRangeIndex);

        switch (bindingType)
        {
        case slang::BindingType::ParameterBlock: {
            slang::TypeLayoutReflection* elementLayout =
                typeLayout->getBindingRangeLeafTypeLayout(bindingRangeIndex)->getElementTypeLayout();

            std::vector<std::string_view> descNames;
            std::vector<std::string_view> subObjNames;
            i32 fieldCount = elementLayout->getFieldCount();
            for (i32 i = 0; i < fieldCount; ++i)
            {
                findParameterNames(elementLayout->getFieldByIndex(i), descNames, subObjNames);
            }

            // Type should be a struct
            if (fieldCount == 0)
            {
                log(LogLevel::WARNING,
                    "CompiledShaderModule::addParameterResources(): Parameter block \"{}\" is not a "
                    "struct and will be ignored",
                    subObjectNames[nameIndex++]);
                continue;
            }

            m_resources.emplace_back();
            std::string prefix = namePrefix + std::string(subObjectNames[nameIndex++]) + ".";
            addParameterBlockRanges(stage, elementLayout, prefix, descNames, subObjNames, m_resources.size() - 1);
        }
        break;
        case slang::BindingType::PushConstant: {
            slang::TypeLayoutReflection* elementTypeLayout =
                typeLayout->getBindingRangeLeafTypeLayout(bindingRangeIndex)->getElementTypeLayout();

            ParameterBinding parameter;
            parameter.name = namePrefix + std::string(subObjectNames[nameIndex++]);
            parameter.shaderStage = stage;
            parameter.offset = m_parameters.empty() ? 0 : m_parameters.back().offset + m_parameters.back().size;
            parameter.size = static_cast<u32>(elementTypeLayout->getSize());
            m_parameters.push_back(parameter);
        }
        break;
        default:
            break;
        }
    }
}

void CompiledShaderModule::findParameterNames(slang::VariableLayoutReflection* varLayout,
                                              std::vector<std::string_view>& descriptorNames,
                                              std::vector<std::string_view>& subObjectNames,
                                              bool isEntryPointParameters)
{
    if (varLayout->getCategory() == slang::ParameterCategory::DescriptorTableSlot)
    {
        descriptorNames.push_back(varLayout->getName());
    }
    else if (varLayout->getCategory() == slang::ParameterCategory::SubElementRegisterSpace ||
             (varLayout->getCategory() == slang::ParameterCategory::Uniform && isEntryPointParameters))
    {
        // Only add ParamterBlock and entry point uniform parameter names
        subObjectNames.push_back(varLayout->getName());
    }
    else if (varLayout->getCategory() == slang::ParameterCategory::Mixed)
    {
        i32 fieldCount = varLayout->getTypeLayout()->getFieldCount();
        for (i32 i = 0; i < fieldCount; ++i)
        {
            findParameterNames(varLayout->getTypeLayout()->getFieldByIndex(i), descriptorNames, subObjectNames);
        }
    }
}

} // namespace huedra