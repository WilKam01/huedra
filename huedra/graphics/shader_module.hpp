#pragma once
#include "core/serialization/json.hpp"
#include "core/types.hpp"
#include "graphics/pipeline_data.hpp"
#include "platform/slang/config.hpp"

namespace huedra {

class ShaderModule
{
public:
    struct EntryPoint
    {
        std::string name;
        ShaderStage stage;
    };

    ShaderModule() = default;
    ~ShaderModule() = default;

    ShaderModule(const ShaderModule& rhs) = default;
    ShaderModule& operator=(const ShaderModule& rhs) = default;
    ShaderModule(ShaderModule&& rhs) = default;
    ShaderModule& operator=(ShaderModule&& rhs) = default;

    void init(Slang::ComPtr<slang::IModule> shaderModule,
              const std::vector<Slang::ComPtr<slang::IEntryPoint>>& entryPoints, const std::vector<SlangStage>& stages);

    std::string getName() const { return m_name; }
    std::string getFullName() const { return m_fullName; }
    std::vector<EntryPoint> getEntryPoints() const { return m_entryPoints; }

    Slang::ComPtr<slang::IModule> getSlangModule() const { return m_module; }
    std::vector<Slang::ComPtr<slang::IEntryPoint>> getSlangEntryPoints() const { return m_slangEntryPoints; }

    ShaderStage getShaderStage(const std::string& entryPoint) const;

private:
    Slang::ComPtr<slang::IModule> m_module;
    std::string m_name;
    std::string m_fullName;
    std::vector<EntryPoint> m_entryPoints;
    std::vector<Slang::ComPtr<slang::IEntryPoint>> m_slangEntryPoints;
};

class CompiledShaderModule
{
public:
    CompiledShaderModule() = default;
    ~CompiledShaderModule() = default;

    void init(Slang::ComPtr<slang::IComponentType> program, const u8* code, u64 codeLen);

    CompiledShaderModule(const CompiledShaderModule& rhs) = default;
    CompiledShaderModule& operator=(const CompiledShaderModule& rhs) = default;
    CompiledShaderModule(CompiledShaderModule&& rhs) = default;
    CompiledShaderModule& operator=(CompiledShaderModule&& rhs) = default;

    std::optional<ResourcePosition> getResource(std::string_view name) const;
    std::optional<ParameterBinding> getParameter(std::string_view name) const;

    const std::vector<u8>& getCode() const { return m_code; }
    const std::vector<std::vector<ResourceBinding>>& getResources() const { return m_resources; }
    const std::vector<ParameterBinding>& getParameters() const { return m_parameters; }

    // Metal specific getters
    const std::vector<std::vector<ResourceBinding>>& getMetalBuffer() const { return m_metalBuffers; }
    const std::vector<ResourceBinding>& getMetalTextures() const { return m_metalTextures; }
    const std::vector<ResourceBinding>& getMetalSamplers() const { return m_metalSamplers; }

    JsonObject getJson() const;

private:
    void addParameterBlockRangesVulkan(ShaderStage stage, slang::TypeLayoutReflection* typeLayout,
                                       const std::string& namePrefix,
                                       const std::vector<std::string_view>& descriptorNames,
                                       const std::vector<std::string_view>& subObjectNames, u32 setIndex);
    void findParameterNamesVulkan(slang::VariableLayoutReflection* varLayout,
                                  std::vector<std::string_view>& descriptorNames,
                                  std::vector<std::string_view>& subObjectNames, bool isEntryPointParameters = false);

    void addParametersMetal(ShaderStage stage, const std::string& namePrefix,
                            slang::VariableLayoutReflection* varLayout, slang::TypeLayoutReflection* typeLayout,
                            bool isEntryPointParameters = false);
    void addResourcesMetal(ShaderStage stage, const std::string& name, ResourceType type,
                           slang::ParameterCategory category);

    std::vector<u8> m_code;
    std::vector<std::vector<ResourceBinding>> m_resources;
    std::vector<ParameterBinding> m_parameters;

    // Metal specific data
    std::vector<std::vector<ResourceBinding>> m_metalBuffers;
    std::vector<ResourceBinding> m_metalTextures;
    std::vector<ResourceBinding> m_metalSamplers;

    struct MetalParameter
    {
        u32 startOffset{0};
        u32 bufferIndex{0};
    };
    std::map<ShaderStage, MetalParameter> m_metalParameters; // Per shader stage, will use separate buffer
};

struct ShaderInput
{
    ShaderModule* shaderModule;
    std::string entryPointName;
};

} // namespace huedra