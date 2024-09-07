#pragma once

#include "core/types.hpp"
#include "math/vec2.hpp"
#include "math/vec3.hpp"

namespace huedra {

struct MeshData
{
    std::string name;
    std::vector<vec3> positions;
    std::vector<vec3> normals;
    std::vector<vec2> uvs;
    std::vector<u32> indices;
};

} // namespace huedra