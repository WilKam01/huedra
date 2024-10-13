#pragma once

#include "core/log.hpp"
#include "core/memory/huffman_tree.hpp"
#include "core/types.hpp"

#include <bit>
#include <cstring>

namespace huedra {

template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
constexpr T convertToHost(T value, std::endian origEndian)
{
    if (std::endian::native == origEndian)
    {
        return value;
    }
    return std::byteswap(value);
}

template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
constexpr T parseFromBytes(const u8* bytes, std::endian origEndian)
{
    T value;
    std::memcpy(&value, bytes, sizeof(T));
    return convertToHost<T>(value, origEndian);
}

template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
constexpr void parseToBytes(u8* bytes, T value, std::endian desiredEndian)
{
    value = convertToHost(value, desiredEndian);
    std::memcpy(bytes, &value, sizeof(T));
}

// LSB: Least Significant BIt
constexpr u32 readBits(const u8* bytes, u32 startBit, u32 length, bool lsbFirst = true)
{
    u32 result = 0;
    u32 byteOffset = startBit / 8;
    u32 bitPos = startBit - byteOffset * 8; // Same as startBit % 8

    u32 bitsRead = 0;
    while (bitsRead < length)
    {
        u32 bitsInCur = std::min(8 - bitPos, length - bitsRead);

        if (lsbFirst)
        {
            u8 mask = ((1u << bitsInCur) - 1) << bitPos;
            u8 extractedBits = (bytes[byteOffset] & mask) >> bitPos;
            result |= extractedBits << bitsRead;
        }
        else
        {
            u8 mask = ((1u << bitsInCur) - 1) << (8 - bitPos - bitsInCur);
            u8 extractedBits = (bytes[byteOffset] & mask) >> (8 - bitPos - bitsInCur);
            result |= extractedBits << (length - bitsRead - bitsInCur);
        }

        bitsRead += bitsInCur;
        ++byteOffset;
        bitPos = 0; // After first loop, start from the first bit
    }

    return result;
}

// Decompression of the deflate algorithm using LZ77 and Huffman coding
constexpr std::vector<u8> inflate(const u8* bytes)
{
    u64 index = 0;

    // compression method and flags
    u8 cmf = bytes[index];

    u8 compressionMethod = readBits(&cmf, 0, 4);
    if (compressionMethod != 8) // "deflate" method
    {
        log(LogLevel::WARNING, "inflate(): Incorrect compression method used: %d (should be 8)", compressionMethod);
        return std::vector<u8>();
    }

    u8 compressionInfo = readBits(&cmf, 4, 4);
    if (compressionInfo > 7) // compression info = log2(windowSize) - 8
    {
        log(LogLevel::WARNING, "inflate(): Compression info used: %d is too large (should be 7 or less)",
            compressionInfo);
        return std::vector<u8>();
    }

    u8 flags = bytes[index + 1];
    if ((cmf * 256 + flags) % 31 != 0) // Has to be multiple of 31
    {
        log(LogLevel::WARNING, "inflate(): cmf and flags 16-bit representation is not a multiple of 31");
        return std::vector<u8>();
    }

    bool dictionaryPresent = static_cast<bool>(readBits(&flags, 5, 1));
    if (dictionaryPresent)
    {
        index += 4; // Skip dictionary
    }

    index += 2;

    // Read blocks
    bool finalBlock = false;
    u64 bits = 0;
    std::vector<u8> retData;

    // Official length and distance codes
    constexpr std::array<u32, 29> lengthExtraBits{0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2,
                                                  2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
    constexpr std::array<u32, 29> lengthBase{3,  4,  5,  6,  7,  8,  9,  10, 11,  13,  15,  17,  19,  23, 27,
                                             31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};

    constexpr std::array<u32, 30> distanceExtraBits{0, 0, 0, 0, 1, 1, 2, 2,  3,  3,  4,  4,  5,  5,  6,
                                                    6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};
    constexpr std::array<u32, 30> distanceBase{1,    2,    3,    4,    5,    7,    9,    13,    17,    25,
                                               33,   49,   65,   97,   129,  193,  257,  385,   513,   769,
                                               1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577};
    while (!finalBlock)
    {
        // Read Block Header
        finalBlock = static_cast<bool>(readBits(&bytes[index], bits++, 1));
        u32 blockType = readBits(&bytes[index], bits, 2);
        bits += 2;

        if (blockType == 0) // No compression
        {
            bits += 8 - (bits % 8); // Skip until nex byte
            u64 offset = bits / 8;

            u16 len = parseFromBytes<u16>(&bytes[index + offset], std::endian::little);
            u16 nLen = parseFromBytes<u16>(&bytes[index + offset + 2], std::endian::little);
            if (len != ~nLen)
            {
                log(LogLevel::WARNING, "inflate(): len/nLen in uncompressed data block is invalid/corrupt");
                return std::vector<u8>();
            }

            u64 start = retData.size();
            retData.resize(start + len);
            std::memcpy(&retData[start], &bytes[index + offset + 4], len);
            bits += (len + 4) * 8;
        }
        else if (blockType == 1 || blockType == 2) // Compressed
        {
            HuffmanTree literalLenTree;
            HuffmanTree distanceTree;
            if (blockType == 1) // Fixed Huffman codes
            {
                // Set everything 8 bits since 0-143 and 280-287 is 8, override the rest
                u64 i = 144;
                std::vector<u32> codes(288, 8);
                for (; i < 256; ++i)
                {
                    codes[i] = 9;
                }
                for (; i < 280; ++i)
                {
                    codes[i] = 7;
                }
                literalLenTree.init(codes, 286);

                codes.assign(30, 5);
                distanceTree.init(codes, 30);
            }
            else // Dynamic Huffman codes
            {
                // "Interesting" code length order
                std::array<u8, 19> codeLenOrder{16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

                u32 hLit = readBits(&bytes[index], bits, 5) + 257;
                bits += 5;

                u32 hDist = readBits(&bytes[index], bits, 5) + 1;
                bits += 5;

                u32 hcLen = readBits(&bytes[index], bits, 4) + 4;
                bits += 4;

                std::vector<u32> codes(19, 0);
                for (u32 i = 0; i < hcLen; ++i)
                {
                    codes[codeLenOrder[i]] = readBits(&bytes[index], bits, 3);
                    bits += 3;
                }

                HuffmanTree codeLenTree;
                codeLenTree.init(codes, 19);

                codes.clear();
                while (codes.size() < static_cast<u64>(hLit + hDist))
                {
                    u32 symbol = codeLenTree.decodeSymbol(&bytes[index], bits);
                    if (symbol >= 0 && symbol <= 15)
                    {
                        codes.push_back(symbol);
                    }
                    else if (symbol == 16)
                    {
                        // Repeat last code [3, 6] times (using 2 additional bits to describe value in range)
                        u32 lastCode = codes.back();
                        u32 repeat = readBits(&bytes[index], bits, 2) + 3;
                        bits += 2;
                        codes.resize(codes.size() + repeat, lastCode);
                    }
                    else if (symbol == 17)
                    {
                        // Repeat code 0 [3, 10] times (using 3 additional bits to describe value in range)
                        u32 repeat = readBits(&bytes[index], bits, 3) + 3;
                        bits += 3;
                        codes.resize(codes.size() + repeat, 0);
                    }
                    else if (symbol == 18)
                    {
                        // Repeat code 0 [11, 138] times (using 7 additional bits to describe value in range)
                        u32 repeat = readBits(&bytes[index], bits, 7) + 11;
                        bits += 7;
                        codes.resize(codes.size() + repeat, 0);
                    }
                    else
                    {
                        log(LogLevel::WARNING,
                            "inflate(): symbol read of dynamic huffman codes is invalid: %d, should be [0, 18]",
                            symbol);
                        return std::vector<u8>();
                    }
                }

                auto itSplit = codes.begin() + hLit;
                std::vector litCodes(codes.begin(), itSplit);
                std::vector distCodes(itSplit, codes.end());

                literalLenTree.init(litCodes, 286);
                distanceTree.init(distCodes, 30);
            }

            u32 symbol = 0;
            while (symbol != 256) // End of block
            {
                symbol = literalLenTree.decodeSymbol(&bytes[index], bits);
                if (symbol < 256)
                {
                    retData.push_back(symbol);
                }
                else if (symbol > 256)
                {
                    symbol -= 257;

                    u32 len = readBits(&bytes[index], bits, lengthExtraBits[symbol]) + lengthBase[symbol];
                    bits += lengthExtraBits[symbol];

                    u32 distSymbol = distanceTree.decodeSymbol(&bytes[index], bits);
                    u32 dist = readBits(&bytes[index], bits, distanceExtraBits[distSymbol]) + distanceBase[distSymbol];
                    bits += distanceExtraBits[distSymbol];

                    u64 origSize = retData.size();
                    u64 curIndex = origSize - dist;
                    for (u64 i = 0; i < len; ++i)
                    {
                        retData.push_back(retData[curIndex++]);
                        if (curIndex >= origSize)
                        {
                            curIndex -= dist;
                        }
                    }
                }
            }
        }
        else
        {
            log(LogLevel::WARNING, "inflate(): Block Type is %d is undefined/reserved", blockType);
            return std::vector<u8>();
        }
    }

#ifdef DEBUG
    // Calculate ADLER32 checksum
    u64 curLen = retData.size();
    u64 offset = 0;
    u32 s1 = 1;
    u32 s2 = 0;

    // Do maximum number of bytes where mod 65521 is unnecessary
    while (curLen >= 5552)
    {
        curLen -= 5552;
        for (u64 i = 0; i < 5552; i += 16) // Do 16 at a time for less loop overhead (loop unrolling)
        {
            s1 += retData[offset + i];
            s2 += s1;
            s1 += retData[offset + i + 1];
            s2 += s1;
            s1 += retData[offset + i + 2];
            s2 += s1;
            s1 += retData[offset + i + 3];
            s2 += s1;

            s1 += retData[offset + i + 4];
            s2 += s1;
            s1 += retData[offset + i + 5];
            s2 += s1;
            s1 += retData[offset + i + 6];
            s2 += s1;
            s1 += retData[offset + i + 7];
            s2 += s1;

            s1 += retData[offset + i + 8];
            s2 += s1;
            s1 += retData[offset + i + 9];
            s2 += s1;
            s1 += retData[offset + i + 10];
            s2 += s1;
            s1 += retData[offset + i + 11];
            s2 += s1;

            s1 += retData[offset + i + 12];
            s2 += s1;
            s1 += retData[offset + i + 13];
            s2 += s1;
            s1 += retData[offset + i + 14];
            s2 += s1;
            s1 += retData[offset + i + 15];
            s2 += s1;
        }

        s1 %= 65521;
        s2 %= 65521;
        offset += 5552;
    }

    // If any left, do the rest
    for (u64 i = offset; i < retData.size(); ++i)
    {
        s1 += retData[i];
        s2 += s1;
    }

    s1 %= 65521;
    s2 %= 65521;

    u32 adler32 = parseFromBytes<u32>(&bytes[index + bits / 8 + 1], std::endian::big);
    if (s2 * 65536 + s1 != adler32)
    {
        log(LogLevel::WARNING, "inflate(): ADLER32: %u is not accurate (calculated value: %u)", adler32,
            s2 * 65536 + s1);
    }
#endif

    return retData;
}

} // namespace huedra
