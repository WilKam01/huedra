#include "loader.hpp"
#include "core/file/utils.hpp"
#include "core/memory/utils.hpp"
#include "math/vec3.hpp"

#include <cmath>

namespace huedra {

TextureData loadPng(const std::string& path, TexelChannelFormat desiredFormat)
{
    TextureData textureData;
    std::vector<u8> bytes = readBytes(path);

    static constexpr std::array<u32, 256> crcTable = {
        {0x0,        0x77073096, 0xee0e612c, 0x990951ba, 0x76dc419,  0x706af48f, 0xe963a535, 0x9e6495a3, 0xedb8832,
         0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x9b64c2b,  0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
         0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7, 0x136c9856, 0x646ba8c0, 0xfd62f97a,
         0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
         0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3,
         0x45df5c75, 0xdcd60dcf, 0xabd13d59, 0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
         0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab,
         0xb6662d3d, 0x76dc4190, 0x1db7106,  0x98d220bc, 0xefd5102a, 0x71b18589, 0x6b6b51f,  0x9fbfe4a5, 0xe8b8d433,
         0x7807c9a2, 0xf00f934,  0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x86d3d2d,  0x91646c97, 0xe6635c01, 0x6b6b51f4,
         0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
         0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65, 0x4db26158, 0x3ab551ce, 0xa3bc0074,
         0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
         0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525,
         0x206f85b3, 0xb966d409, 0xce61e49f, 0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
         0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x3b6e20c,  0x74b1d29a, 0xead54739, 0x9dd277af, 0x4db2615,
         0x73dc1683, 0xe3630b12, 0x94643b84, 0xd6d6a3e,  0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0xa00ae27,  0x7d079eb1,
         0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7, 0xfed41b76,
         0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
         0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b, 0xd80d2bda, 0xaf0a1b4c, 0x36034af6,
         0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
         0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7,
         0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d, 0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x26d930a,  0x9c0906a9, 0xeb0e363f,
         0x72076785, 0x5005713,  0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0xcb61b38,  0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7,
         0xbdbdf21,  0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
         0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45, 0xa00ae278,
         0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
         0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9, 0xbdbdf21c, 0xcabac28a, 0x53b39330,
         0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
         0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d}};

    // Check for png signature
    constexpr u64 pngSignature = 0x89504e470d0a1a0a; // 137, 80, 78, 71, 13, 10, 26, 10
    if (parseFromBytes<u64>(bytes.data(), std::endian::big) != pngSignature)
    {
        log(LogLevel::WARNING, "loadPng(): {} does not have a valid png signature", path.c_str());
        return textureData;
    }

    struct HeaderInfo
    {
        u32 width{0};
        u32 height{0};
        u8 bitDepth{0};
        enum class ColorType
        {
            GRAYSCALE = 0,
            TRUECOLOR = 2,
            INDEXED_COLOR = 3,
            GRAYSCALE_ALPHA = 4,
            TRUECOLOR_ALPHA = 6
        } colorType{ColorType::GRAYSCALE};
        u8 compressionMethod{0};
        u8 filterMethod{0};
        u8 interlaceMethod{0};
    } header;
    const std::array<u8, 7> CHANNELS_PER_TYPE{1, 0, 3, 1, 2, 0, 4};
    std::vector<uvec3> colorPalette;
    std::vector<u8> imageBytes;
    u32 channelSize = 0;
    u32 numChannels = 0;

    // Read chunks
    for (u64 i = 8; i < bytes.size();)
    {
        u32 chunkLen = parseFromBytes<u32>(&bytes[i], std::endian::big);
        std::string chunkType(4, '\0');
        i += 4;
        chunkType[0] = static_cast<char>(bytes[i]);
        chunkType[1] = static_cast<char>(bytes[i + 1]);
        chunkType[2] = static_cast<char>(bytes[i + 2]);
        chunkType[3] = static_cast<char>(bytes[i + 3]);

        u32 calcCrc = ~0u; // Max value of u32
        for (u64 j = 0; j < chunkLen + 4; ++j)
        {
            calcCrc = crcTable[(calcCrc ^ bytes[i + j]) & 0xff] ^ (calcCrc >> 8);
        }
        i += 4;
        u32 crc = parseFromBytes<u32>(&bytes[i + chunkLen], std::endian::big);

        if (calcCrc != ~crc)
        {
            log(LogLevel::WARNING, "loadPng(): CRC for chunk: {} is invalid (c = {}, crc = {})", chunkType.c_str(),
                calcCrc, crc);
            return {};
        }

        // Header
        if (chunkType == "IHDR")
        {
            header.width = parseFromBytes<u32>(&bytes[i], std::endian::big);
            header.height = parseFromBytes<u32>(&bytes[i + 4], std::endian::big);

            header.bitDepth = bytes[i + 8];
            if (header.bitDepth != 1 && header.bitDepth != 2 && header.bitDepth != 4 && header.bitDepth != 8 &&
                header.bitDepth != 16)
            {
                log(LogLevel::WARNING, "loadPng(): Incorrect bit depth in IHDR: {}", header.bitDepth);
                return {};
            }

            u8 colorType = bytes[i + 9];
            if (colorType == 1 || colorType == 5 || colorType > 6)
            {
                log(LogLevel::WARNING, "loadPng(): Incorrect colorType in IHDR: {}", colorType);
                return {};
            }
            header.colorType = static_cast<HeaderInfo::ColorType>(colorType);

            header.compressionMethod = bytes[i + 10];
            if (header.compressionMethod != 0)
            {
                log(LogLevel::WARNING, "loadPng(): Incorrect compression method in IHDR: {}", header.compressionMethod);
                return {};
            }

            header.filterMethod = bytes[i + 11];
            if (header.filterMethod != 0)
            {
                log(LogLevel::WARNING, "loadPng(): Incorrect filter method in IHDR: {}", header.filterMethod);
                return {};
            }

            header.interlaceMethod = bytes[i + 12];
            if (header.interlaceMethod == 1)
            {
                log(LogLevel::WARNING, "loadPng(): Interlace method 1 (Adam7) not supported");
                return {};
            }
            if (header.interlaceMethod != 0)
            {
                log(LogLevel::WARNING, "loadPng(): Incorrect interlace method in IHDR: {}", header.interlaceMethod);
                return {};
            }

            textureData.width = header.width;
            textureData.height = header.height;
            channelSize = header.bitDepth <= 8 ? 1 : 2;
            numChannels = static_cast<u32>(desiredFormat);
            textureData.texelSize = numChannels * channelSize;

            switch (desiredFormat)
            {
            case TexelChannelFormat::G:
                textureData.format = channelSize == 1 ? GraphicsDataFormat::R_8_UNORM : GraphicsDataFormat::R_16_UNORM;
                break;
            case TexelChannelFormat::GA:
                textureData.format =
                    channelSize == 1 ? GraphicsDataFormat::RG_8_UNORM : GraphicsDataFormat::RG_16_UNORM;
                break;
            case TexelChannelFormat::RGB:
                textureData.format =
                    channelSize == 1 ? GraphicsDataFormat::RGB_8_UNORM : GraphicsDataFormat::RGB_16_UNORM;
                break;
            case TexelChannelFormat::RGBA:
                textureData.format =
                    channelSize == 1 ? GraphicsDataFormat::RGBA_8_UNORM : GraphicsDataFormat::RGBA_16_UNORM;
                break;
            }
        }
        // Color Palette
        else if (chunkType == "PLTE")
        {
            if (chunkLen % 3 != 0)
            {
                log(LogLevel::WARNING, "loadPng(): PLTE chunk length is not divisible by 3");
                return {};
            }
            colorPalette.resize(chunkLen / 3);
            for (u64 j = 0; j < colorPalette.size(); ++j)
            {
                colorPalette[j].r = bytes[i + (j * 3)];
                colorPalette[j].g = bytes[i + (j * 3) + 1];
                colorPalette[j].b = bytes[i + (j * 3) + 2];
            }
        }
        // Image Data
        else if (chunkType == "IDAT")
        {
            imageBytes.insert(imageBytes.end(), bytes.begin() + static_cast<i64>(i),
                              bytes.begin() + static_cast<i64>(i + chunkLen));
        }
        // End Chunk
        else if (chunkType == "IEND")
        {
            break;
        }
        // Unkown or not supported
        else
        {
            // If first char is uppercase (5th bit == 1) the chunk is critical
            // Critical chunks has to be supported for correct decoding of png image data
            bool isCritical = static_cast<bool>((readBits(reinterpret_cast<u8*>(chunkType.data()), 5, 1)) == 0);
            if (isCritical)
            {
                log(LogLevel::WARNING, "loadPng(): Chunk type: {} is critical but not supported, aborting load",
                    chunkType.c_str());
                return {};
            }
            log(LogLevel::D_INFO, "loadPng(): Ancilliary Chunk type: {} not supported, will be ignored",
                chunkType.c_str());
        }

        i += chunkLen + 4;
    }

    if (imageBytes.empty())
    {
        log(LogLevel::WARNING, "loadPng(): No IDAT chunks present");
        return {};
    }

    // Inflate image data
    imageBytes = inflate(imageBytes.data());

    if (imageBytes.empty())
    {
        return {};
    }

    // Allocate texels
    textureData.texels.resize(static_cast<u64>(textureData.width) * static_cast<u64>(textureData.height) *
                              static_cast<u64>(textureData.texelSize));

    // Reconstruct image data for each scanline (reverse filtering)
    float bytesPerPixel = (static_cast<float>(header.bitDepth) / 8.0f) *
                          static_cast<float>(CHANNELS_PER_TYPE[static_cast<u64>(header.colorType)]);
    u64 scanlineByteWidth = static_cast<u64>(std::round(static_cast<float>(header.width) * bytesPerPixel) + 1);
    u64 wholeBytesPerPixel = static_cast<u64>(std::round(bytesPerPixel)); // Sets it to 1 if less for use in filtering
    for (u64 i = 0; i < header.height; ++i)
    {
        u8 filterType = imageBytes[i * scanlineByteWidth];
        if (filterType > 4)
        {
            log(LogLevel::WARNING, "loadPng(): filter type: {} is not valid for filter method 0", filterType);
            return {};
        }
        for (u64 j = 0; j < scanlineByteWidth - 1; ++j)
        {
            // In the filter types the letters a, b, c mean the following

            // a: channel before current channel in scanline
            // b: channel in the same position as current channel on the previous scanline
            // c: channel before current channel on the previous scanline (basically a but if b was current byte)

            // Can be imaginged as the following matrix:
            // |c b|
            // |a x|
            // where x is the current color channel

            u64 curIndex = (i * scanlineByteWidth) + j + 1;
            u8 a = 0;
            u8 b = 0;
            u8 c = 0;
            switch (filterType)
            {
            case 0: // None
                break;
            case 1: // Sub
                a = static_cast<i64>(j - wholeBytesPerPixel) < 0 ? 0 : imageBytes[curIndex - wholeBytesPerPixel];
                imageBytes[curIndex] += a;
                break;
            case 2: // Up
                b = i == 0 ? 0 : imageBytes[curIndex - scanlineByteWidth];
                imageBytes[curIndex] += b;
                break;
            case 3: // Average
                a = static_cast<i64>(j - wholeBytesPerPixel) < 0 ? 0 : imageBytes[curIndex - wholeBytesPerPixel];
                b = i == 0 ? 0 : imageBytes[curIndex - scanlineByteWidth];
                imageBytes[curIndex] += static_cast<u8>((static_cast<u16>(a) + static_cast<u16>(b)) / 2);
                break;
            case 4: { // Paeth
                a = static_cast<i64>(j - wholeBytesPerPixel) < 0 ? 0 : imageBytes[curIndex - wholeBytesPerPixel];
                b = i == 0 ? 0 : imageBytes[curIndex - scanlineByteWidth];
                c = i == 0 || static_cast<i64>(j - wholeBytesPerPixel) < 0
                        ? 0
                        : imageBytes[curIndex - scanlineByteWidth - wholeBytesPerPixel];

                i16 p = static_cast<i16>(static_cast<i16>(a) + static_cast<i16>(b) - static_cast<i16>(c));
                u16 pa = std::abs(p - static_cast<i16>(a));
                u16 pb = std::abs(p - static_cast<i16>(b));
                u16 pc = std::abs(p - static_cast<i16>(c));
                if (pa <= pb && pa <= pc)
                {
                    imageBytes[curIndex] += a;
                }
                else if (pb <= pc)
                {
                    imageBytes[curIndex] += b;
                }
                else
                {
                    imageBytes[curIndex] += c;
                }
            }
            break;
            default:
                break;
            }
        }

        u32 byteIndex = 1;
        u64 bits = 0;
        for (u64 j = 0; j < textureData.width; ++j)
        {
            std::vector<u16> pngChannels;
            if (header.colorType == HeaderInfo::ColorType::INDEXED_COLOR)
            {
                pngChannels.resize(3, 0);
                u8 index = readBits(&imageBytes[(i * scanlineByteWidth) + byteIndex], bits, header.bitDepth, false);
                bits += header.bitDepth;
                pngChannels[0] = colorPalette[index].r;
                pngChannels[1] = colorPalette[index].g;
                pngChannels[2] = colorPalette[index].b;
            }
            else
            {
                pngChannels.resize(CHANNELS_PER_TYPE[static_cast<u64>(header.colorType)], 0);
                for (auto& pngChannel : pngChannels)
                {
                    if (header.bitDepth == 8)
                    {
                        pngChannel =
                            parseFromBytes<u8>(&imageBytes[(i * scanlineByteWidth) + byteIndex++], std::endian::big);
                    }
                    else if (header.bitDepth == 16)
                    {
                        pngChannel =
                            parseFromBytes<u16>(&imageBytes[(i * scanlineByteWidth) + byteIndex], std::endian::big);
                        byteIndex += 2;
                    }
                    else // 1, 2, 4 bits
                    {
                        pngChannel = static_cast<u16>(
                            static_cast<float>(readBits(&imageBytes[(i * scanlineByteWidth) + byteIndex], bits,
                                                        header.bitDepth, false)) *
                            255.0 / static_cast<float>((1u << header.bitDepth) - 1));
                        bits += header.bitDepth;
                    }
                }
            }

            bool srcHasAlpha = header.colorType == HeaderInfo::ColorType::GRAYSCALE_ALPHA ||
                               header.colorType == HeaderInfo::ColorType::TRUECOLOR_ALPHA;
            bool srcIsGrayscale = header.colorType == HeaderInfo::ColorType::GRAYSCALE ||
                                  header.colorType == HeaderInfo::ColorType::GRAYSCALE_ALPHA;
            bool dstHasAlpha = desiredFormat == TexelChannelFormat::GA || desiredFormat == TexelChannelFormat::RGBA;
            bool dstIsGrayscale = desiredFormat == TexelChannelFormat::G || desiredFormat == TexelChannelFormat::GA;

            u16 alpha = pngChannels.back();
            if (srcIsGrayscale && !dstIsGrayscale)
            {
                pngChannels.assign(3, pngChannels[0]);
            }
            else if (!srcIsGrayscale && dstIsGrayscale)
            {
                u16 val = static_cast<u16>((static_cast<u32>(pngChannels[0]) + static_cast<u32>(pngChannels[1]) +
                                            static_cast<u32>(pngChannels[2])) /
                                           3);
                pngChannels.assign(1, val);
            }

            if (srcHasAlpha && dstHasAlpha)
            {
                pngChannels.push_back(alpha);
            }
            else if (!srcHasAlpha && dstHasAlpha)
            {
                pngChannels.push_back(0xffff); // Max u16 value
            }

            u64 scanlineStart = i * textureData.width * textureData.texelSize;
            u64 texelStart = j * textureData.texelSize;
            for (u64 k = 0; k < numChannels; ++k)
            {
                if (header.bitDepth == 16)
                {
                    parseToBytes<u16>(&textureData.texels[scanlineStart + texelStart + (k * 2)], pngChannels[k],
                                      std::endian::native);
                }
                else
                {
                    parseToBytes<u8>(&textureData.texels[scanlineStart + texelStart + k], pngChannels[k],
                                     std::endian::native);
                }
            }
        }
    }

    return textureData;
}

} // namespace huedra
