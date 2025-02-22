#pragma once

#include "resources/mesh/data.hpp"

namespace huedra {

std::vector<MeshData> loadObj(const std::string& path);

std::vector<MeshData> loadGltf(const std::string& path);

std::vector<MeshData> loadGlb(const std::string& path);

}