#pragma once

#include "core/types.hpp"
#include "graphics/shader_module.hpp"
#include "platform/slang/config.hpp"

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

    ShaderModule createModule(const std::string& name, const std::string& source);
    CompiledShaderModule compileAndLinkModules(const std::vector<ShaderModule>& modules);

private:
    Slang::ComPtr<slang::IGlobalSession> m_globalSession;
    Slang::ComPtr<slang::ISession> m_session;
};

} // namespace huedra
