#include "shader_module.hpp"

#include "core/file/utils.hpp"
#include "core/global.hpp"
#include "core/log.hpp"
#include "core/serialization/json.hpp"
#include "core/string/utils.hpp"
#include "graphics/pipeline_data.hpp"

#include <algorithm>
#include <array>
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

ShaderStage ShaderModule::getShaderStage(const std::string& entryPointName) const
{
    ShaderStage stage = ShaderStage::NONE;

    auto it = std::ranges::find_if(m_entryPoints,
                                   [entryPointName](const EntryPoint& entry) { return entry.name == entryPointName; });
    if (it != m_entryPoints.end())
    {
        stage = it->stage;
    }

    return stage;
}

Slang::ComPtr<slang::IEntryPoint> ShaderModule::getSlangEntryPoint(const std::string& entryPointName) const
{
    Slang::ComPtr<slang::IEntryPoint> entryPoint;
    m_module->findEntryPointByName(entryPointName.c_str(), entryPoint.writeRef());
    return entryPoint;
}

// TODO: add support for resource arrays
void CompiledShaderModule::init(Slang::ComPtr<slang::IComponentType> program, const u8* code, u64 codeLen)
{
    m_code = std::vector<u8>(code, code + codeLen);
    slang::ProgramLayout* programLayout = program->getLayout();

#ifdef VULKAN
    m_resources.emplace_back(); // First set
    std::vector<std::string_view> descriptorNames;
    std::vector<std::string_view> subObjectNames;

    u32 parameterCount = programLayout->getParameterCount();
    for (u32 i = 0; i < parameterCount; ++i)
    {
        findParameterNamesVulkan(programLayout->getParameterByIndex(i), descriptorNames, subObjectNames);
    }

    addParameterBlockRangesVulkan(ShaderStage::ALL, programLayout->getGlobalParamsTypeLayout(), "", descriptorNames,
                                  subObjectNames, 0);

    u32 entryPointCount = programLayout->getEntryPointCount();
    for (u32 i = 0; i < entryPointCount; ++i)
    {
        descriptorNames.clear();
        subObjectNames.clear();

        slang::EntryPointReflection* entryPointLayout = programLayout->getEntryPointByIndex(i);
        parameterCount = entryPointLayout->getParameterCount();
        for (u32 i = 0; i < parameterCount; ++i)
        {
            findParameterNamesVulkan(entryPointLayout->getParameterByIndex(i), descriptorNames, subObjectNames, true);
        }

        addParameterBlockRangesVulkan(convertSlangShaderStage(entryPointLayout->getStage()),
                                      entryPointLayout->getTypeLayout(), "", descriptorNames, subObjectNames, 0);

        if (convertSlangShaderStage(entryPointLayout->getStage()) == ShaderStage::COMPUTE)
        {
            std::array<SlangUInt, 3> threadValues{{}};
            entryPointLayout->getComputeThreadGroupSize(3, threadValues.data());
            m_computeThreadsPerGroup.x = threadValues[0];
            m_computeThreadsPerGroup.y = threadValues[1];
            m_computeThreadsPerGroup.z = threadValues[2];
        }
    }
#elif defined(METAL)
    u32 parameterCount = programLayout->getParameterCount();
    for (u32 i = 0; i < parameterCount; ++i)
    {
        addParametersMetal(ShaderStage::ALL, "", programLayout->getParameterByIndex(i),
                           programLayout->getGlobalParamsTypeLayout());
    }

    u32 entryPointCount = programLayout->getEntryPointCount();
    for (u32 i = 0; i < entryPointCount; ++i)
    {
        slang::EntryPointReflection* entryPointLayout = programLayout->getEntryPointByIndex(i);
        parameterCount = entryPointLayout->getParameterCount();
        for (u32 i = 0; i < parameterCount; ++i)
        {
            addParametersMetal(convertSlangShaderStage(entryPointLayout->getStage()), "",
                               entryPointLayout->getParameterByIndex(i), entryPointLayout->getTypeLayout(), true);
        }

        if (convertSlangShaderStage(entryPointLayout->getStage()) == ShaderStage::COMPUTE)
        {
            std::array<SlangUInt, 3> threadValues{{}};
            entryPointLayout->getComputeThreadGroupSize(3, threadValues.data());
            m_computeThreadsPerGroup.x = threadValues[0];
            m_computeThreadsPerGroup.y = threadValues[1];
            m_computeThreadsPerGroup.z = threadValues[2];
        }
    }

#endif
}

std::optional<ResourcePosition> CompiledShaderModule::getResource(std::string_view name) const
{
#ifdef METAL
    // Look for buffer or part of argument buffer
    for (u32 i = 0; i < m_metalBuffers.size(); ++i)
    {
        for (u32 j = 0; j < m_metalBuffers[i].size(); ++j)
        {
            if (m_metalBuffers[i][j].name == name)
            {
                ResourcePosition resource;
                resource.info = m_metalBuffers[i][j];
                resource.set = i;
                resource.binding = j;
                return resource;
            }
        }
    }

    // Look for texture
    for (u32 i = 0; i < m_metalTextures.size(); ++i)
    {
        if (m_metalTextures[i].name == name)
        {
            ResourcePosition resource;
            resource.info = m_metalTextures[i];
            resource.set = i;
            resource.binding = 0;
            return resource;
        }
    }

    // Look for sampler
    for (u32 i = 0; i < m_metalSamplers.size(); ++i)
    {
        if (m_metalSamplers[i].name == name)
        {
            ResourcePosition resource;
            resource.info = m_metalSamplers[i];
            resource.set = i;
            resource.binding = 0;
            return resource;
        }
    }

#else
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
#endif

    return std::nullopt;
}

std::optional<ParameterBinding> CompiledShaderModule::getParameter(std::string_view name) const
{
    auto it = std::ranges::find_if(m_parameters, [name](const ParameterBinding& param) { return param.name == name; });
    if (it != m_parameters.end())
    {
        return *it;
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

void CompiledShaderModule::addParameterBlockRangesVulkan(ShaderStage stage, slang::TypeLayoutReflection* typeLayout,
                                                         const std::string& namePrefix,
                                                         const std::vector<std::string_view>& descriptorNames,
                                                         const std::vector<std::string_view>& subObjectNames,
                                                         u32 setIndex)
{
    if (typeLayout->getSize() > 0)
    {
        ResourceBinding resource;
        resource.name = namePrefix + "<misc>";
        resource.shaderStage = stage;
        resource.type = ResourceType::CONSTANT_BUFFER;
        m_resources[setIndex].push_back(resource);
    }

    u32 rangeCount = typeLayout->getDescriptorSetDescriptorRangeCount(0);
    u32 nameIndex = 0;
    for (u32 i = 0; i < rangeCount; ++i)
    {
        slang::BindingType bindingType = typeLayout->getDescriptorSetDescriptorRangeType(0, i);

        // Skip push constants
        if (bindingType == slang::BindingType::PushConstant)
        {
            continue;
        }

        if (nameIndex >= descriptorNames.size())
        {
            log(LogLevel::D_INFO,
                "addParameterBlockRangesVulkan(): Current name index is outside of descriptor names range ({} >= {})",
                nameIndex, descriptorNames.size());
            return;
        }
        ResourceBinding resource;
        resource.name = namePrefix + std::string(descriptorNames[nameIndex++]);
        resource.shaderStage = stage;
        resource.type = convertSlangResourceType(bindingType);
        m_resources[setIndex].push_back(resource);
    }

    rangeCount = typeLayout->getSubObjectRangeCount();
    nameIndex = 0;
    for (u32 i = 0; i < rangeCount; ++i)
    {
        u32 bindingRangeIndex = typeLayout->getSubObjectRangeBindingRangeIndex(i);
        slang::BindingType bindingType = typeLayout->getBindingRangeType(bindingRangeIndex);

        switch (bindingType)
        {
        case slang::BindingType::ParameterBlock: {
            slang::TypeLayoutReflection* elementLayout =
                typeLayout->getBindingRangeLeafTypeLayout(bindingRangeIndex)->getElementTypeLayout();

            std::vector<std::string_view> descNames;
            std::vector<std::string_view> subObjNames;
            u32 fieldCount = elementLayout->getFieldCount();
            for (u32 i = 0; i < fieldCount; ++i)
            {
                findParameterNamesVulkan(elementLayout->getFieldByIndex(i), descNames, subObjNames);
            }

            // Type should be a struct
            if (fieldCount == 0)
            {
                log(LogLevel::WARNING,
                    "addParameterBlockRangesVulkan(): Parameter block \"{}\" is not a struct and will be ignored",
                    subObjectNames[nameIndex++]);
                continue;
            }

            if (nameIndex >= subObjectNames.size())
            {
                log(LogLevel::D_INFO,
                    "addParameterBlockRangesVulkan(): Current name index is outside of subobject names names range ({} "
                    ">= {})",
                    nameIndex, subObjectNames.size());
                return;
            }
            m_resources.emplace_back();
            std::string prefix = namePrefix + std::string(subObjectNames[nameIndex++]) + ".";
            addParameterBlockRangesVulkan(stage, elementLayout, prefix, descNames, subObjNames, m_resources.size() - 1);
        }
        break;
        case slang::BindingType::PushConstant: {
            slang::TypeLayoutReflection* elementTypeLayout =
                typeLayout->getBindingRangeLeafTypeLayout(bindingRangeIndex)->getElementTypeLayout();

            ParameterBinding parameter;
            parameter.name = namePrefix + std::string(subObjectNames[nameIndex++]);
            parameter.shaderStage = stage;
            parameter.offset = m_nextParameterOffset;
            parameter.size = static_cast<u32>(elementTypeLayout->getSize());

            m_parameters.push_back(parameter);
            m_nextParameterOffset += parameter.size;
        }
        break;
        default:
            break;
        }
    }
}

void CompiledShaderModule::findParameterNamesVulkan(slang::VariableLayoutReflection* varLayout,
                                                    std::vector<std::string_view>& descriptorNames,
                                                    std::vector<std::string_view>& subObjectNames,
                                                    bool isEntryPointParameters)
{
    if (varLayout->getCategory() == slang::ParameterCategory::DescriptorTableSlot)
    {
        descriptorNames.emplace_back(varLayout->getName());
    }
    else if (varLayout->getCategory() == slang::ParameterCategory::SubElementRegisterSpace ||
             (varLayout->getCategory() == slang::ParameterCategory::Uniform && isEntryPointParameters))
    {
        // Only add ParamterBlock and entry point uniform parameter names
        subObjectNames.emplace_back(varLayout->getName());
    }
    else if (varLayout->getCategory() == slang::ParameterCategory::Mixed)
    {
        u32 fieldCount = varLayout->getTypeLayout()->getFieldCount();
        for (u32 i = 0; i < fieldCount; ++i)
        {
            findParameterNamesVulkan(varLayout->getTypeLayout()->getFieldByIndex(i), descriptorNames, subObjectNames);
        }
    }
}

void CompiledShaderModule::addParametersMetal(ShaderStage stage, const std::string& namePrefix,
                                              slang::VariableLayoutReflection* varLayout,
                                              slang::TypeLayoutReflection* typeLayout, bool isEntryPointParameters)
{
    switch (varLayout->getTypeLayout()->getBindingRangeType(0))
    {
    case slang::BindingType::ConstantBuffer: {
        u32 fieldCount = varLayout->getTypeLayout()->getFieldCount();
        // Type is a struct
        if (fieldCount != 0)
        {
            std::string prefix = namePrefix + std::string(varLayout->getName()) + ".";
            for (u32 i = 0; i < fieldCount; ++i)
            {
                addParametersMetal(stage, prefix, varLayout->getTypeLayout()->getFieldByIndex(i),
                                   varLayout->getTypeLayout());
            }
            break;
        }
    }
    case slang::BindingType::Texture:
    case slang::BindingType::MutableTexture:
    case slang::BindingType::Sampler: {
        addResourcesMetal(stage, namePrefix + varLayout->getName(),
                          convertSlangResourceType(varLayout->getTypeLayout()->getBindingRangeType(0)),
                          varLayout->getCategory());
    }
    break;
    case slang::BindingType::ParameterBlock: {
        u32 fieldCount = varLayout->getTypeLayout()->getElementTypeLayout()->getFieldCount();
        // Type should be a struct
        if (fieldCount == 0)
        {
            log(LogLevel::WARNING,
                "CompiledShaderModule::addParametersMetal(): Parameter block \"{}\" is not a "
                "struct and will be ignored",
                varLayout->getName());
            break;
        }

        m_resources.emplace_back();    // Add set for argument buffer
        m_metalBuffers.emplace_back(); // Add set for argument buffer
        std::string prefix = namePrefix + std::string(varLayout->getName()) + ".";
        for (u32 i = 0; i < fieldCount; ++i)
        {
            addParametersMetal(stage, prefix, varLayout->getTypeLayout()->getElementTypeLayout()->getFieldByIndex(i),
                               typeLayout->getBindingRangeLeafTypeLayout(0)->getElementTypeLayout());
        }
    }
    break;
    case slang::BindingType::Unknown:
        if (varLayout->getCategory() == slang::ParameterCategory::Uniform && isEntryPointParameters)
        {
            ParameterBinding parameter;
            parameter.name = varLayout->getName();
            parameter.shaderStage = stage;
            parameter.offset = m_nextParameterOffset;
            parameter.size = static_cast<u32>(varLayout->getTypeLayout()->getSize());

            // Start of new shader stage, add starting offset (since metal uses separate buffers per entrypoint)
            ShaderStage curStage = m_parameters.empty() ? ShaderStage::NONE : m_parameters.back().shaderStage;
            if (curStage != parameter.shaderStage)
            {
                // Find first instance of a buffer that is not this shader stage
                auto it =
                    std::ranges::find_if(m_metalBuffers, [&parameter](const std::vector<ResourceBinding>& buffers) {
                        return buffers[0].shaderStage == parameter.shaderStage;
                    });

                ResourceBinding paramBuffer;
                paramBuffer.name = std::string("__") +
                                   std::string(ShaderStageNames[static_cast<u64>(parameter.shaderStage)]) +
                                   std::string("__");
                paramBuffer.shaderStage = parameter.shaderStage;
                paramBuffer.type = ResourceType::NONE; // Use none to indicate parameters

                MetalShaderStageParametersInfo param;
                param.startByte = m_nextParameterOffset;

                // Did not find any other instance of this shader stage
                if (it == m_metalBuffers.end())
                {
                    param.bufferIndex = m_metalBuffers.empty() ? 0 : m_metalBuffers.size();
                    m_metalParameters.insert(
                        std::pair<ShaderStage, MetalShaderStageParametersInfo>(parameter.shaderStage, param));
                    m_metalBuffers.push_back({paramBuffer});
                }
                // Did find first instance of a buffer in this shader stage, push parameters in front
                else
                {
                    param.bufferIndex = std::distance(m_metalBuffers.begin(), it);
                    m_metalParameters.insert(
                        std::pair<ShaderStage, MetalShaderStageParametersInfo>(parameter.shaderStage, param));
                    m_metalBuffers.insert(it, {paramBuffer});
                }
            }

            m_parameters.push_back(parameter);
            m_nextParameterOffset += parameter.size;
        }
        // Add automatically created constant buffer
        else if (!getResource(namePrefix + "<misc>").has_value() &&
                 std::string(varLayout->getSemanticName()).substr(0, 3) != "SV_")
        {
            addResourcesMetal(stage, namePrefix + "<misc>", ResourceType::CONSTANT_BUFFER, varLayout->getCategory());
        }
        break;
    default:
        break;
    }
}

void CompiledShaderModule::addResourcesMetal(ShaderStage stage, const std::string& name, ResourceType type,
                                             slang::ParameterCategory category)
{
    ResourceBinding resource;
    resource.name = name;
    resource.shaderStage = stage;
    resource.type = type;

    // Should not need to look at None, but seems that some resources are not set correctly when in Parameter Block
    if (category == slang::ParameterCategory::MetalArgumentBufferElement || category == slang::ParameterCategory::None)
    {
        resource.partOfArgBuffer = true;
        m_resources.back().push_back(resource);
        m_metalBuffers.back().push_back(resource);
    }
    else
    {
        m_resources.push_back({resource});
        if (resource.type == ResourceType::CONSTANT_BUFFER || resource.type == ResourceType::STRUCTURED_BUFFER)
        {
            m_metalBuffers.push_back({resource});
        }
        else if (resource.type == ResourceType::TEXTURE || resource.type == ResourceType::RW_TEXTURE)
        {
            m_metalTextures.push_back(resource);
        }
        else if (resource.type == ResourceType::SAMPLER)
        {
            m_metalSamplers.push_back(resource);
        }
    }
}

} // namespace huedra