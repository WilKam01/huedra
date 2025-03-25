#pragma once

#include "core/file/types.hpp"
#include "core/log.hpp"
#include "core/string/utils.hpp"

#include <fstream>

namespace huedra {

inline std::vector<u8> readBytes(const std::string& path)
{
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        log(LogLevel::ERR, "Failed to open file: \"{}\"!", path.c_str());
        return {};
    }

    u64 size = static_cast<u64>(file.tellg());
    if (size == 0)
    {
        return {};
    }

    std::vector<u8> buffer(size, 0);

    file.seekg(0);
    file.read(reinterpret_cast<char*>(buffer.data()), static_cast<i64>(size));
    file.close();

    return buffer;
}

inline bool writeBytes(const std::string& path, const std::vector<u8>& bytes)
{
    std::ofstream file(path);
    if (!file.is_open())
    {
        log(LogLevel::ERR, "Failed to open file: \"{}\"!", path.c_str());
        return false;
    }

    file.write(reinterpret_cast<const char*>(bytes.data()), static_cast<i64>(bytes.size()));
    file.close();
    return true;
}

inline FilePathInfo transformFilePath(const std::string& path)
{
    FilePathInfo filePathInfo{};
    filePathInfo.directories = splitByChars(path, "/\\");

    std::array<std::string, 2> nameAndExtension = splitFirstByChar(filePathInfo.directories.back(), '.');
    filePathInfo.fileName = nameAndExtension[0];
    filePathInfo.extension = nameAndExtension[1];
    if (filePathInfo.extension.empty())
    {
        log(LogLevel::WARNING, "transformFilePath(): No extension found for %s", path.c_str());
    }

    // Keep only directories
    filePathInfo.directories.pop_back();

    return filePathInfo;
}

} // namespace huedra