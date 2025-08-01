#pragma once

#include "core/types.hpp"
#include "math/vec2.hpp"
#include <vector>

namespace huedra {

struct GlyphPoint
{
    ivec2 position{0};
    bool onCurve{false};
};

struct GlyphContourRange
{
    u16 start{0};
    u16 end{0};
};

struct Glyph
{
    i16vec2 min{0};
    i16vec2 max{0};
    bool isSimple{false};
    u16 advanceWidth{0};
    i16 leftSideBearing{0};
    std::vector<GlyphContourRange> contourRanges;
    std::vector<GlyphPoint> points;
};

struct FontData
{
    std::map<u16, u32> characterMappings; // std::map<unicode character, glyphIndex>
    std::vector<Glyph> glyphs;
    u16 unitsPerEm{0};
};

} // namespace huedra