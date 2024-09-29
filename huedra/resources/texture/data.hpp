#pragma once

#include "core/types.hpp"
#include "graphics/pipeline_data.hpp"

namespace huedra {

struct TextureData
{
    u32 width{0};
    u32 height{0};
    GraphicsDataFormat format{GraphicsDataFormat::UNDEFINED};
    u32 texelSize{0};
    std::vector<u8> texels{};
};

} // namespace huedra
