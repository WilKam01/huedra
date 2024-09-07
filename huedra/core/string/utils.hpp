#pragma once

#include "core/types.hpp"
#include <sstream>

namespace huedra {

inline std::vector<std::string> splitByChar(const std::string& str, char delim)
{
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;

    while (std::getline(ss, token, delim))
    {
        tokens.push_back(token);
    }

    return tokens;
}

} // namespace huedra