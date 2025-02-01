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

// Splits only string by the last instance of delim
inline std::vector<std::string> splitLastByChar(const std::string& str, char delim)
{
    std::vector<std::string> tokens;
    u64 i = str.length() - 1;
    while (str[i--] != delim && i >= 0)
    {
    }

    if (i < 0)
    {
        tokens.push_back(str);
    }
    else
    {
        tokens.push_back(str.substr(0, i + 1));
        tokens.push_back(str.substr(i + 2));
    }

    return tokens;
}

template <typename T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
inline std::string toHex(T value)
{
    return std::format("{:x}", value);
}

} // namespace huedra