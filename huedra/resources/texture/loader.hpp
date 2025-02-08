#pragma once

#include "core/types.hpp"
#include "resources/texture/data.hpp"

namespace huedra {

TextureData loadPng(const std::string& path, TexelChannelFormat desiredFormat);

} // namespace huedra
