#include "shader_module.hpp"

namespace huedra {

void ShaderModule::init(const u8* code, u64 codeLength) { m_code = std::vector<u8>(code, code + codeLength); }

void ShaderModule::cleanup() { m_code.clear(); }

} // namespace huedra