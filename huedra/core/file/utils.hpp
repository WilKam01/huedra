#pragma once

#include "huedra/core/file/types.hpp"
#include "huedra/core/log.hpp"
#include "huedra/core/string/utils.hpp"

#include <fstream>

namespace huedra {

inline std::vector<u8> readBytes(std::string_view path)
{
    std::ifstream file(path.data(), std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        log::func::error("Failed to open file: \"{}\"!", path);
        return {};
    }

    u64 size = static_cast<u64>(file.tellg());
    if (size == 0)
    {
        return {};
    }

    std::vector<u8> buffer(size, 0);

    file.seekg(0);
    file.read(std::bit_cast<char*>(buffer.data()), static_cast<i64>(size));
    file.close();

    return buffer;
}

inline bool writeBytes(std::string_view path, const std::vector<u8>& bytes)
{
    std::ofstream file(path.data());
    if (!file.is_open())
    {
        log::func::error("Failed to open file: \"{}\"!", path);
        return false;
    }

    file.write(std::bit_cast<const char*>(bytes.data()), static_cast<i64>(bytes.size()));
    file.close();
    return true;
}

inline FilePathInfo transformFilePath(std::string_view path)
{
    FilePathInfo filePathInfo{};
    std::array<std::string, 2> split = splitLastByChars(path, "/\\");
    filePathInfo.dirPath = split[0];

    std::array<std::string, 2> nameAndExtension = splitFirstByChar(split[1], '.');
    filePathInfo.fileName = nameAndExtension[0];
    filePathInfo.extension = nameAndExtension[1];
    if (filePathInfo.extension.empty())
    {
        log::func::warn("No extension found for {}", path);
    }

    return filePathInfo;
}

} // namespace huedra