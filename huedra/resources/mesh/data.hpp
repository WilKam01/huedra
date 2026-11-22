#pragma once

#include "huedra/core/types.hpp"
#include "huedra/math/vec2.hpp"
#include "huedra/math/vec3.hpp"

namespace huedra {

struct MeshData
{
    std::string name;
    std::vector<vec3> positions;
    std::vector<vec2> uvs;
    std::vector<vec3> normals;
    std::vector<u32> indices;
};

} // namespace huedra