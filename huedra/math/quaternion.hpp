#pragma once

#include "math/vec3.hpp"
#include "math/vec_transform.hpp"

namespace huedra {

class Quaternion
{
public:
    Quaternion() = default;
    constexpr Quaternion(float scalar, const vec3& axis) : scalar(scalar), axis(axis) {}

    constexpr Quaternion operator+(const Quaternion& rhs) const { return {scalar + rhs.scalar, axis + rhs.axis}; }
    constexpr Quaternion operator*(const Quaternion& rhs) const
    {
        return {(scalar * rhs.scalar) - math::dot(axis, rhs.axis),
                (scalar * rhs.axis) + (rhs.scalar * axis) + math::cross(axis, rhs.axis)};
    }

    constexpr Quaternion operator*(float scalar) const { return {this->scalar * scalar, axis * scalar}; }
    constexpr Quaternion operator/(float scalar) const { return {this->scalar / scalar, axis / scalar}; }
    constexpr Quaternion operator-() const { return {-scalar, -axis}; }

    float scalar{0.0f};
    vec3 axis;
};

constexpr Quaternion operator*(float scalar, const Quaternion& quat)
{
    return {quat.scalar * scalar, quat.axis * scalar};
}

} // namespace huedra