#pragma once

#include "resources/mesh/data.hpp"

namespace huedra {

std::vector<MeshData> loadObj(std::string_view path);

std::vector<MeshData> loadGltf(std::string_view path);

std::vector<MeshData> loadGlb(std::string_view path);

} // namespace huedra