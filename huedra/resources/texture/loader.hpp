#pragma once

#include "core/types.hpp"
#include "resources/texture/data.hpp"

namespace huedra {

enum class TexelChannelFormat
{
    G = 1,   // Grayscale
    GA = 2,  // Grayscale, Alpha
    RGB = 3, // Red, Green, Blue
    RGBA = 4 // Red, Green, Blue, Alpha
};

TextureData loadPng(const std::string& path, TexelChannelFormat desiredFormat);

} // namespace huedra
