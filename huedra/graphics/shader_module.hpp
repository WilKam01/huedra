#pragma once
#include "core/types.hpp"

namespace huedra {
class ShaderModule
{
public:
    ShaderModule() = default;
    ~ShaderModule() = default;

    void init(const u8* code, u64 codeLength);
    void cleanup();

    ShaderModule(const ShaderModule& rhs) = default;
    ShaderModule& operator=(const ShaderModule& rhs) = default;
    ShaderModule(ShaderModule&& rhs) = default;
    ShaderModule& operator=(ShaderModule&& rhs) = default;

    const std::vector<u8>& getCode() const { return m_code; }

private:
    std::vector<u8> m_code;
};
} // namespace huedra