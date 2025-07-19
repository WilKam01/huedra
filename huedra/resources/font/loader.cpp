#include "loader.hpp"
#include "core/file/utils.hpp"
#include "core/log.hpp"
#include "core/memory/utils.hpp"
#include "core/types.hpp"
#include "resources/font/data.hpp"

#include <array>
#include <bit>
#include <cstring>
#include <string_view>
#include <utility>

namespace huedra {

// Used for true type, post type and open type fonts
struct CommonFontHeader
{
    std::string scalerTypeString;
    u32 scalerType{0};
    u16 numTables{0};
    u16 searchRange{0};
    u16 entrySelector{0};
    u16 rangeShift{0};

    enum class Type
    {
        UNDEFINED,
        TRUE_TYPE,
        POSTSCRIPT,
        OPEN_TYPE
    };
    Type type{Type::UNDEFINED};

    // TODO: Create this automatically? Macro?
    constexpr static std::array<std::string_view, 4> TypeNames{"Undefined", "True Type", "PostScript", "Open Type"};
};

namespace {

// Used for true type, post type and open type fonts
CommonFontHeader loadCommonFontHeader(const std::vector<u8>& bytes)
{
    CommonFontHeader header;
    header.scalerTypeString = {static_cast<char>(bytes[0]), static_cast<char>(bytes[1]), static_cast<char>(bytes[2]),
                               static_cast<char>(bytes[3])};
    header.scalerType = parseFromBytes<u32>(bytes.data(), std::endian::big);
    header.numTables = parseFromBytes<u16>(&bytes[4], std::endian::big);
    header.searchRange = parseFromBytes<u16>(&bytes[6], std::endian::big);
    header.entrySelector = parseFromBytes<u16>(&bytes[8], std::endian::big);
    header.rangeShift = parseFromBytes<u16>(&bytes[10], std::endian::big);

    if (header.scalerTypeString == "true" || header.scalerType == 0x00010000)
    {
        header.type = CommonFontHeader::Type::TRUE_TYPE;
    }
    else if (header.scalerTypeString == "typ1")
    {
        header.type = CommonFontHeader::Type::POSTSCRIPT;
    }
    else if (header.scalerTypeString == "OTTO")
    {
        header.type = CommonFontHeader::Type::OPEN_TYPE;
    }

    return header;
}

} // namespace

FontData loadTtf(const std::string& path)
{
    std::vector<u8> bytes{readBytes(path)};
    CommonFontHeader header = loadCommonFontHeader(bytes);

    if (header.type != CommonFontHeader::Type::TRUE_TYPE)
    {
        log(LogLevel::WARNING, "loadTtf(): Invalid font type, expected {}, but got {}",
            CommonFontHeader::TypeNames[static_cast<u32>(CommonFontHeader::Type::TRUE_TYPE)],
            CommonFontHeader::TypeNames[static_cast<u32>(header.type)]);
        return {};
    }

    struct Table
    {
        u32 checkSum{0};
        u32 offset{0};
        u32 length{0};
    };
    std::map<std::string, Table> tables;

    for (u32 i = 0; i < header.numTables; ++i)
    {
        std::string tag = {static_cast<char>(bytes[12 + (i * 16)]), static_cast<char>(bytes[13 + (i * 16)]),
                           static_cast<char>(bytes[14 + (i * 16)]), static_cast<char>(bytes[15 + (i * 16)])};
        u32 checkSum = parseFromBytes<u32>(&bytes[16 + (i * 16)], std::endian::big);
        u32 offset = parseFromBytes<u32>(&bytes[20 + (i * 16)], std::endian::big);
        u32 length = parseFromBytes<u32>(&bytes[24 + (i * 16)], std::endian::big);

        // Check checksums?

        tables.insert(std::pair<std::string, Table>(tag, {.checkSum = checkSum, .offset = offset, .length = length}));
    }

    // TODO: Only check tables that are used by application?
    // Check validity of font
    constexpr std::array<std::string_view, 9> requiredTables{"cmap", "glyf", "head", "hhea", "hmtx",
                                                             "loca", "maxp", "name", "post"};
    for (const auto& required : requiredTables)
    {
        if (!tables.contains(std::string(required)))
        {
            log(LogLevel::WARNING, "loadTtf(): Missing table name in file: {}", required);
            return {};
        }
    }

    FontData fontData;
    fontData.unitsPerEm = parseFromBytes<u16>(&bytes[tables["head"].offset + 18], std::endian::big);

    i16 indexToLocFormat = parseFromBytes<i16>(&bytes[tables["head"].offset + 50], std::endian::big);
    u32 bytesPerLocLookUp = indexToLocFormat == 0 ? 2 : 4;
    u16 numGlyphs = parseFromBytes<u16>(&bytes[tables["maxp"].offset + 4], std::endian::big);

    std::vector<u32> glyphLocations(numGlyphs, tables["glyf"].offset);
    for (u32 i = 0; i < numGlyphs; ++i)
    {
        u32 readIndex = tables["loca"].offset + (i * bytesPerLocLookUp);
        u32 glyphOffset = 0;
        if (bytesPerLocLookUp == 2)
        {
            glyphOffset = static_cast<u32>(parseFromBytes<u16>(&bytes[readIndex], std::endian::big)) * 2;
        }
        else
        {
            glyphOffset = parseFromBytes<u32>(&bytes[readIndex], std::endian::big);
        }
        glyphLocations[i] += glyphOffset;
    }

    u16 numLongHorMetrics = parseFromBytes<u16>(&bytes[tables["hhea"].offset + 34], std::endian::big);
    struct LongHorMetric
    {
        u16 advanceWidth{0};
        i16 leftSideBearing{0};
    };
    std::vector<LongHorMetric> longHorMetrics{numGlyphs};

    u16 hmtxOffset = tables["hmtx"].offset;
    for (u16 i = 0; i < numLongHorMetrics; ++i)
    {
        longHorMetrics[i].advanceWidth = parseFromBytes<u16>(&bytes[hmtxOffset + (i * 4)], std::endian::big);
        longHorMetrics[i].leftSideBearing = parseFromBytes<i16>(&bytes[hmtxOffset + (i * 4) + 2], std::endian::big);
    }

    // Remaining glyphs are monospace and are using the same advance width as the last in the hMetrics array
    hmtxOffset += numLongHorMetrics * sizeof(LongHorMetric);
    for (u16 i = numLongHorMetrics; i < numGlyphs; ++i)
    {
        longHorMetrics[i].advanceWidth = longHorMetrics[numLongHorMetrics - 1].advanceWidth;
        longHorMetrics[i].leftSideBearing =
            parseFromBytes<i16>(&bytes[hmtxOffset + ((i - numLongHorMetrics) * 2)], std::endian::big);
    }

    u16 numSubtables = parseFromBytes<u16>(&bytes[tables["cmap"].offset + 2], std::endian::big);
    u32 selectedSubtableOffset{0};
    i32 selectedUnicodeVersion{-1};
    // Look through the different subtables and find the best platform
    // Currently we only look for unicode encodings
    for (u16 i = 0; i < numSubtables; ++i)
    {
        u16 platformId = parseFromBytes<u16>(&bytes[tables["cmap"].offset + 4 + (i * 8)], std::endian::big);
        u16 platformSpecificId = parseFromBytes<u16>(&bytes[tables["cmap"].offset + 4 + (i * 8) + 2], std::endian::big);
        u32 offset = parseFromBytes<u32>(&bytes[tables["cmap"].offset + 4 + (i * 8) + 4], std::endian::big);

        // Unicode platform
        if (platformId == 0)
        {
            if ((platformSpecificId == 0 || platformSpecificId == 1 || platformSpecificId == 3) &&
                selectedUnicodeVersion < platformSpecificId)
            {
                selectedSubtableOffset = offset;
                selectedUnicodeVersion = platformSpecificId;
            }
        }
        // Windows platform
        else if (platformId == 3)
        {
            // Only use this if valid Unicode platform doesn't exist
            if ((platformSpecificId == 1 || platformSpecificId == 10) && selectedUnicodeVersion != -1)
            {
                selectedSubtableOffset = offset;
            }
        }
    }

    if (selectedSubtableOffset == 0)
    {
        log(LogLevel::WARNING, "loadTtf(): Did not find suitable encoding in \"cmap\" table");
        return {};
    }

    u16 format = parseFromBytes<u16>(&bytes[tables["cmap"].offset + selectedSubtableOffset], std::endian::big);
    if (format != 4 && format != 6 && format != 12 && format != 13)
    {
        log(LogLevel::WARNING, "loadTtf(): Selected subtable format: {} is not supported", format);
        return {};
    }

    u32 curOffset = tables["cmap"].offset + selectedSubtableOffset;
    if (format == 4)
    {
        u16 segCount = parseFromBytes<u16>(&bytes[curOffset + 6], std::endian::big) / 2;

        struct Segment
        {
            u16 endCode;
            u16 startCode;
            u16 idDelta;
            u16 idRangeOffset;
        };
        std::vector<Segment> segments(segCount);
        curOffset += 14;

        // Last segment should be 0xffff on start code and end code, ignore it
        for (u16 i = 0; i < segCount - 1; ++i)
        {
            u32 offset = curOffset + (i * 2);
            u16 endCode = parseFromBytes<u16>(&bytes[offset], std::endian::big);
            offset += segCount * 2 + 2;
            u16 startCode = parseFromBytes<u16>(&bytes[offset], std::endian::big);
            offset += segCount * 2;
            u16 idDelta = parseFromBytes<u16>(&bytes[offset], std::endian::big);
            offset += segCount * 2;
            u16 idRangeOffset = parseFromBytes<u16>(&bytes[offset], std::endian::big);

            // Calculate code directly
            if (idRangeOffset == 0)
            {
                // Go through all characters in the range
                for (u16 i = startCode; i <= endCode; ++i)
                {
                    fontData.characterMappings[i] = (i + idDelta) % 65536;
                }
            }
            // Look up in glyph index array
            else
            {
                u16 rangeOffsetLocation = offset + idRangeOffset;

                // Go through all characters in the range
                for (u16 i = startCode; i <= endCode; ++i)
                {
                    u16 glyphIndexOffset = (2 * (i - startCode)) + rangeOffsetLocation;
                    u16 index = parseFromBytes<u16>(&bytes[glyphIndexOffset], std::endian::big);
                    if (index != 0)
                    {
                        fontData.characterMappings[i] = (index + idDelta) % 65536;
                    }
                }
            }
        }
    }
    else if (format == 6)
    {
        u16 firstCode = parseFromBytes<u16>(&bytes[curOffset + 6], std::endian::big);
        u16 entryCount = parseFromBytes<u16>(&bytes[curOffset + 8], std::endian::big);
        curOffset += 8;
        for (u16 i = 0; i < entryCount; ++i)
        {
            fontData.characterMappings[firstCode + i] =
                parseFromBytes<u16>(&bytes[curOffset + (i * 2)], std::endian::big);
        }
    }
    else if (format == 12)
    {
        u32 numGroups = parseFromBytes<u32>(&bytes[curOffset + 10], std::endian::big);
        curOffset += 14;
        for (u32 i = 0; i < numGroups; ++i)
        {
            u32 startCharCode = parseFromBytes<u32>(&bytes[curOffset + (i * 12)], std::endian::big);
            u32 endCharCode = parseFromBytes<u32>(&bytes[curOffset + (i * 12) + 4], std::endian::big);
            u32 startGlyphCode = parseFromBytes<u32>(&bytes[curOffset + (i * 12) + 8], std::endian::big);

            for (u32 i = startCharCode; i <= endCharCode; ++i)
            {
                fontData.characterMappings[i] = startGlyphCode + (i - startCharCode);
            }
        }
    }
    else if (format == 13)
    {
        u32 numGroups = parseFromBytes<u32>(&bytes[curOffset + 10], std::endian::big);
        curOffset += 14;
        for (u32 i = 0; i < numGroups; ++i)
        {
            u32 startCharCode = parseFromBytes<u32>(&bytes[curOffset + (i * 12)], std::endian::big);
            u32 endCharCode = parseFromBytes<u32>(&bytes[curOffset + (i * 12) + 4], std::endian::big);
            u32 startGlyphCode = parseFromBytes<u32>(&bytes[curOffset + (i * 12) + 8], std::endian::big);

            for (u32 i = startCharCode; i <= endCharCode; ++i)
            {
                fontData.characterMappings[i] = startGlyphCode;
            }
        }
    }

    fontData.glyphs.resize(numGlyphs);
    for (u32 i = 0; i < numGlyphs; ++i)
    {
        u32 curByteIndex = glyphLocations[i];
        i16 numberOfContours = parseFromBytes<i16>(&bytes[curByteIndex], std::endian::big);
        Glyph glyph;
        glyph.minSize.x = parseFromBytes<i16>(&bytes[curByteIndex + 2], std::endian::big);
        glyph.minSize.y = parseFromBytes<i16>(&bytes[curByteIndex + 4], std::endian::big);
        glyph.maxSize.x = parseFromBytes<i16>(&bytes[curByteIndex + 6], std::endian::big);
        glyph.maxSize.y = parseFromBytes<i16>(&bytes[curByteIndex + 8], std::endian::big);
        glyph.isSimple = numberOfContours >= 0;
        glyph.advanceWidth = longHorMetrics[i].advanceWidth;
        glyph.leftSideBearing = longHorMetrics[i].leftSideBearing;

        curByteIndex += 10;
        // Simple glyphs
        if (glyph.isSimple)
        {
            glyph.contourEndPointIndices.resize(static_cast<u64>(numberOfContours));
            for (u32 j = 0; j < static_cast<u32>(numberOfContours); ++j)
            {
                glyph.contourEndPointIndices[j] = parseFromBytes<u16>(&bytes[curByteIndex + (j * 2)], std::endian::big);
            }

            u16 numPoints =
                parseFromBytes<u16>(&bytes[curByteIndex + ((numberOfContours - 1) * 2)], std::endian::big) + 1;
            glyph.points.resize(numPoints);

            // Skip instructionLength and instructions
            u16 instructionLength =
                parseFromBytes<u16>(&bytes[curByteIndex + (numberOfContours * 2)], std::endian::big);
            curByteIndex += (numberOfContours * 2) + 2 + instructionLength;

            std::vector<u8> flags(numPoints, 0);
            enum FlagBits
            {
                FLAG_ON_CURVE,
                FLAG_X_SHORT_VECTOR,
                FLAG_Y_SHORT_VECTOR,
                FLAG_REPEAT,
                FLAG_X_SPECIAL,
                FLAG_Y_SPECIAL
            };
            for (u32 j = 0; j < numPoints; ++j)
            {
                u8 flag = bytes[curByteIndex++];
                flags[j] = flag;

                bool repeat = static_cast<bool>(readBits(&flag, FLAG_REPEAT, 1));
                if (repeat)
                {
                    u8 numRepeats = bytes[curByteIndex++];
                    std::memset(&flags[j + 1], flag, numRepeats);
                    j += numRepeats;
                }
            }

            // xCoordinates
            i32 prevCoord = 0;
            for (u32 j = 0; j < numPoints; ++j)
            {
                glyph.points[j].onCurve = static_cast<bool>(readBits(&flags[j], FLAG_ON_CURVE, 1));

                bool is8Bit = static_cast<bool>(readBits(&flags[j], FLAG_X_SHORT_VECTOR, 1));
                bool specialBit = static_cast<bool>(readBits(&flags[j], FLAG_X_SPECIAL, 1));

                if (is8Bit)
                {
                    u8 coord = bytes[curByteIndex++];
                    i32 actualCoord = static_cast<i32>(coord) * (specialBit ? 1 : -1);
                    glyph.points[j].position.x = prevCoord + actualCoord;
                    prevCoord += actualCoord;
                }
                else if (specialBit) // Same coordinate as previous
                {
                    glyph.points[j].position.x = prevCoord;
                }
                else
                {
                    i16 coord = parseFromBytes<i16>(&bytes[curByteIndex], std::endian::big);
                    curByteIndex += 2;
                    glyph.points[j].position.x = prevCoord + coord;
                    prevCoord += coord;
                }
            }

            // yCoordinates
            prevCoord = 0;
            for (u32 j = 0; j < numPoints; ++j)
            {
                bool is8Bit = static_cast<bool>(readBits(&flags[j], FLAG_Y_SHORT_VECTOR, 1));
                bool specialBit = static_cast<bool>(readBits(&flags[j], FLAG_Y_SPECIAL, 1));

                if (is8Bit)
                {
                    u8 coord = bytes[curByteIndex++];
                    i32 actualCoord = static_cast<i32>(coord) * (specialBit ? 1 : -1);
                    glyph.points[j].position.y = prevCoord + actualCoord;
                    prevCoord += actualCoord;
                }
                else if (specialBit) // Same coordinate as previous
                {
                    glyph.points[j].position.y = prevCoord;
                }
                else
                {
                    i16 coord = parseFromBytes<i16>(&bytes[curByteIndex], std::endian::big);
                    curByteIndex += 2;
                    glyph.points[j].position.y = prevCoord + coord;
                    prevCoord += coord;
                }
            }
        }
        // Compound glyphs
        else
        {
            // TODO: Parse
        }

        fontData.glyphs[i] = glyph;
    }

    return fontData;
}

} // namespace huedra