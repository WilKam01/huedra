#pragma once

#include "core/types.hpp"
#include "math/vec2.hpp"

namespace huedra {

struct Glyph
{
    i16vec2 minSize{0};
    i16vec2 maxSize{0};
    bool isCompound{false};
    u16 advanceWidth{0};
    i16 leftSideBearing{0};
};

struct FontData
{
    std::map<u16, Glyph> glyphs;
    u16 unitsPerEm{0};
};

} // namespace huedra