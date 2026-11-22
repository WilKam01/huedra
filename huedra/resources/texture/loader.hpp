#pragma once

#include "huedra/core/types.hpp"
#include "huedra/resources/texture/data.hpp"

namespace huedra {

TextureData loadPng(std::string_view path, TexelChannelFormat desiredFormat);

HdrTextureData loadRadianceHdr(std::string_view path, TexelChannelFormat desiredFormat);

} // namespace huedra
