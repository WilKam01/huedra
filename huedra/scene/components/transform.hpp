#pragma once

#include "math/matrix_transform.hpp"

namespace huedra {

struct Transform
{
    vec3 position{0.0f};
    vec3 rotation{0.0f};
    vec3 scale{1.0f};

    matrix4 applyMatrix()
    {
        return math::translate(matrix4(1.0f), position) * math::rotateZ(matrix4(1.0f), rotation.z) *
               math::rotateY(matrix4(1.0f), rotation.y) * math::rotateX(matrix4(1.0f), rotation.x) *
               math::scale(matrix4(1.0f), scale);
    }
};

} // namespace huedra
