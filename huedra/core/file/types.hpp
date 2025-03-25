#pragma once

#include "core/types.hpp"

namespace huedra {

struct FilePathInfo
{
    std::vector<std::string> directories;
    std::string fileName;
    std::string extension;
};

} // namespace huedra
