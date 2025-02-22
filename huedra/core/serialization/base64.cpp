#include "base64.hpp"

#include "core/log.hpp"

namespace huedra {

std::vector<u8> decodeBase64(const std::string& base64)
{
    u64 len = base64.length();
    if (len % 4 != 0)
    {
        log(LogLevel::WARNING, "decodeBase64(): incorrect string length: {}", len);
        return {};
    }

    auto decodeChar = [](char c) -> u8 {
        if (c >= 'A' && c <= 'Z')
        {
            return c - 'A';
        }
        if (c >= 'a' && c <= 'z')
        {
            return c - 'a' + 26;
        }
        if (c >= '0' && c <= '9')
        {
            return c - '0' + 52;
        }
        if (c == '+')
        {
            return 62;
        }
        if (c == '/')
        {
            return 63;
        }
        if (c == '=')
        {
            return 64; // Padding
        }
        return 65; // Incorrect
    };

    std::vector<u8> bytes;

    for (u64 i = 0; i < len; i += 4)
    {
        std::array<u8, 4> arr{decodeChar(base64[i]), decodeChar(base64[i + 1]), decodeChar(base64[i + 2]),
                              decodeChar(base64[i + 3])};
        for (u64 j = 0; j < 4; ++j)
        {
            if (arr[j] == 65)
            {
                log(LogLevel::WARNING, "decodeBase64(): Incorrect base64 character: {}", base64[i + j]);
                return {};
            }
        }

        bytes.push_back((arr[0] << 2) | (arr[1] >> 4));

        if (arr[2] != 64)
        {
            bytes.push_back(((arr[1] & 0x0f) << 4) | (arr[2] >> 2));
        }

        if (arr[3] != 64)
        {
            bytes.push_back(((arr[2] & 0x03) << 6) | arr[3]);
        }
    }

    return bytes;
}

} // namespace huedra