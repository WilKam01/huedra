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
        return translate(matrix4(1.0f), position) * rotateZ(matrix4(1.0f), rotation.z) *
               rotateY(matrix4(1.0f), rotation.y) * rotateX(matrix4(1.0f), rotation.x) * scaleMat(matrix4(1.0f), scale);
    }
};

struct Test
{};

} // namespace huedra
