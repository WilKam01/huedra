#pragma once
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
    static ShaderStage convertShaderStage(SlangStage shaderStage);

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

    const std::vector<u8>& getCode() const { return m_code; }

private:
    std::vector<u8> m_code;
};

struct ShaderInput
{
    ShaderModule* shaderModule;
    std::string entryPointName;
};

} // namespace huedra