#include "loader.hpp"
#include "core/file/utils.hpp"
#include "core/log.hpp"
#include "core/memory/utils.hpp"
#include "core/types.hpp"
#include "resources/font/data.hpp"

#include <array>
#include <bit>
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

    log(LogLevel::D_INFO, "loadTtf(): Header");
    log(LogLevel::D_INFO, "    Scaler Type: {} (0x{:x})", header.scalerTypeString.c_str(), header.scalerType);
    log(LogLevel::D_INFO, "    Number of Tables: {}", header.numTables);
    log(LogLevel::D_INFO, "    Search Range: {}", header.searchRange);
    log(LogLevel::D_INFO, "    Entry Selector: {}", header.entrySelector);
    log(LogLevel::D_INFO, "    Range Shift: {}", header.rangeShift);

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

    log(LogLevel::D_INFO, "loadTtf(): Tables");
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

        log(LogLevel::D_INFO, "    Table {}", i);
        log(LogLevel::D_INFO, "        Tag: \"{}\"", tag.c_str());
        log(LogLevel::D_INFO, "        Check Sum: {}", checkSum);
        log(LogLevel::D_INFO, "        Offset: {}", offset);
        log(LogLevel::D_INFO, "        Length: {}", length);
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

    std::vector<u32> glyphLocations(numGlyphs, 0);
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
        glyphLocations[i] = tables["glyf"].offset + glyphOffset;
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
        longHorMetrics[i].advanceWidth =
            parseFromBytes<u16>(&bytes[hmtxOffset + (i * sizeof(LongHorMetric))], std::endian::big);
        longHorMetrics[i].leftSideBearing =
            parseFromBytes<i16>(&bytes[hmtxOffset + (i * sizeof(LongHorMetric)) + 2], std::endian::big);
    }

    // Remaining glyphs are monospace and are using the same advance width as the last in the hMetrics array
    hmtxOffset += numLongHorMetrics * sizeof(LongHorMetric);
    for (u16 i = numLongHorMetrics; i < numGlyphs; ++i)
    {
        longHorMetrics[i].advanceWidth = longHorMetrics[numLongHorMetrics - 1].advanceWidth;
        longHorMetrics[i].leftSideBearing =
            parseFromBytes<i16>(&bytes[hmtxOffset + ((i - numLongHorMetrics) * sizeof(i16))], std::endian::big);
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

        log(LogLevel::D_INFO, "loadTtf(): Subtable {}", i);
        log(LogLevel::D_INFO, "    Platform Id {}", platformId);
        log(LogLevel::D_INFO, "    Platform Specific Id {}", platformSpecificId);
        log(LogLevel::D_INFO, "    Offset {}", offset);
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
    std::map<u16, u16> glyphIndices;
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

        log(LogLevel::D_INFO, "loadTtf(): Segments");
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

            log(LogLevel::D_INFO, "    Segment {}", i);
            log(LogLevel::D_INFO, "        End Code: {}", endCode);
            log(LogLevel::D_INFO, "        Start Code: {}", startCode);
            log(LogLevel::D_INFO, "        Id Delta: {}", idDelta);
            log(LogLevel::D_INFO, "        Id Range Offset: {}", idRangeOffset);

            // Calculate code directly
            if (idRangeOffset == 0)
            {
                // Go through all characters in the range
                for (u16 i = startCode; i <= endCode; ++i)
                {
                    glyphIndices[i] = (i + idDelta) % 65536;
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
                        glyphIndices[i] = (index + idDelta) % 65536;
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
            glyphIndices[firstCode + i] = parseFromBytes<u16>(&bytes[curOffset + (i * 2)], std::endian::big);
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
                glyphIndices[i] = startGlyphCode + (i - startCharCode);
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
                glyphIndices[i] = startGlyphCode;
            }
        }
    }

    for (auto& [charCode, index] : glyphIndices)
    {
        u32 glyphLocation = glyphLocations[index];
        i16 numberOfContours = parseFromBytes<i16>(&bytes[glyphLocation], std::endian::big);
        Glyph glyph;
        glyph.minSize.x = parseFromBytes<i16>(&bytes[glyphLocation + 2], std::endian::big);
        glyph.minSize.y = parseFromBytes<i16>(&bytes[glyphLocation + 4], std::endian::big);
        glyph.maxSize.x = parseFromBytes<i16>(&bytes[glyphLocation + 6], std::endian::big);
        glyph.maxSize.y = parseFromBytes<i16>(&bytes[glyphLocation + 8], std::endian::big);
        glyph.isCompound = numberOfContours < 0;
        glyph.advanceWidth = longHorMetrics[index].advanceWidth;
        glyph.leftSideBearing = longHorMetrics[index].leftSideBearing;
        fontData.glyphs[charCode] = glyph;
    }

    return fontData;
}

} // namespace huedra