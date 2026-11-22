#pragma once

#include "huedra/core/types.hpp"
#include "huedra/graphics/shader_module.hpp"
#include "huedra/platform/slang/config.hpp"

namespace huedra {

class SlangContext
{
public:
    SlangContext() = default;
    ~SlangContext() = default;

    SlangContext(const SlangContext& rhs) = delete;
    SlangContext& operator=(const SlangContext& rhs) = delete;
    SlangContext(SlangContext&& rhs) = delete;
    SlangContext& operator=(SlangContext&& rhs) = delete;

    void init();
    void cleanup();

    ShaderModule createModule(std::string_view name, std::string_view source);
    CompiledShaderModule compileAndLinkModules(const std::map<ShaderStage, ShaderInput>& inputs);

private:
    Slang::ComPtr<slang::IGlobalSession> m_globalSession;
    Slang::ComPtr<slang::ISession> m_session;
};

} // namespace huedra
