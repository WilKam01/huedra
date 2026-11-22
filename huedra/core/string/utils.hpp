#pragma once

#include "huedra/core/types.hpp"
#include <sstream>

namespace huedra {

inline std::vector<std::string> splitByChar(std::string_view str, char delim)
{
    std::vector<std::string> tokens;
    std::stringstream ss(str.data());
    std::string token;

    while (std::getline(ss, token, delim))
    {
        tokens.push_back(token);
    }

    return tokens;
}

inline std::vector<std::string> splitByChars(std::string_view str, std::string_view delims)
{
    std::vector<std::string> tokens;
    u32 i = 0;
    u32 start = 0;
    u32 len = 0;
    while (i < str.length())
    {
        bool anyDelim{false};
        for (auto& delim : delims)
        {
            if (str[i] == delim)
            {
                anyDelim = true;
                break;
            }
        }

        if (anyDelim && len != 0)
        {
            tokens.push_back(std::string(str.substr(start, len)));
            start += len + 1;
            len = 0;
        }
        else
        {
            ++len;
        }
        ++i;
    }

    // Add remainder (if any)
    if (len != 0)
    {
        tokens.push_back(std::string(str.substr(start, len)));
    }

    return tokens;
}

// Splits only string by the first instance of delim
inline std::array<std::string, 2> splitFirstByChar(std::string_view str, char delim)
{
    std::array<std::string, 2> tokens;
    u64 i = 0;
    while (str[i] != delim && i < str.length())
    {
        ++i;
    }

    if (i >= str.length())
    {
        tokens[0] = str;
    }
    else
    {
        tokens[0] = str.substr(0, i);
        tokens[1] = str.substr(i + 1);
    }

    return tokens;
}

// Splits only string by the first instance of delim
inline std::array<std::string, 2> splitFirstByChars(std::string_view str, std::string_view delims)
{
    std::array<std::string, 2> tokens;
    u64 i = 0;
    while (i < str.length())
    {
        bool anyDelim{false};
        for (auto& delim : delims)
        {
            if (str[i] == delim)
            {
                anyDelim = true;
                break;
            }
        }

        if (anyDelim)
        {
            break;
        }
        ++i;
    }

    if (i >= str.length())
    {
        tokens[0] = str;
    }
    else
    {
        tokens[0] = str.substr(0, i);
        tokens[1] = str.substr(i + 1);
    }

    return tokens;
}

// Splits only string by the last instance of delim
inline std::array<std::string, 2> splitLastByChar(std::string_view str, char delim)
{
    std::array<std::string, 2> tokens;
    u64 i = str.length() - 1;
    while (str[i] != delim && i >= 0)
    {
        --i;
    }

    if (i <= 0)
    {
        tokens[0] = str;
    }
    else
    {
        tokens[0] = str.substr(0, i);
        tokens[1] = str.substr(i + 1);
    }

    return tokens;
}

// Splits only string by the last instance of delim
inline std::array<std::string, 2> splitLastByChars(std::string_view str, std::string_view delims)
{
    std::array<std::string, 2> tokens;
    u64 i = str.length() - 1;
    while (i >= 0)
    {
        bool anyDelim{false};
        for (auto& delim : delims)
        {
            if (str[i] == delim)
            {
                anyDelim = true;
                break;
            }
        }

        if (anyDelim)
        {
            break;
        }
        --i;
    }

    if (i <= 0)
    {
        tokens[0] = str;
    }
    else
    {
        tokens[0] = str.substr(0, i);
        tokens[1] = str.substr(i + 1);
    }

    return tokens;
}

} // namespace huedra