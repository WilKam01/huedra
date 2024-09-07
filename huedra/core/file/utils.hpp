#pragma once

#include "core/log.hpp"
#include "core/types.hpp"

#include <fstream>

namespace huedra {

inline std::vector<u8> readBytes(const std::string& path)
{
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        log(LogLevel::ERR, "Failed to open file: \"%s\"!", path.c_str());
        return std::vector<u8>();
    }

    u64 size = static_cast<u64>(file.tellg());
    if (size == 0)
    {
        return std::vector<u8>();
    }

    std::vector<u8> buffer(size, 0);

    file.seekg(0);
    file.read(reinterpret_cast<char*>(buffer.data()), size);
    file.close();

    return std::move(buffer);
}

} // namespace huedra