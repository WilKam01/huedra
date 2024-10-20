#pragma once

#include "math/quaternion.hpp"

namespace huedra {

float dot(const Quaternion& lhs, const Quaternion& rhs) { return lhs.scalar * rhs.scalar + dot(lhs.axis, rhs.axis); }

float length(const Quaternion& quat) { return sqrt(dot(quat, quat)); }

Quaternion normalize(const Quaternion& quat) { return quat / length(quat); }

Quaternion conjugate(const Quaternion& quat) { return Quaternion(quat.scalar, -quat.axis); }

Quaternion inverse(const Quaternion& quat) { return conjugate(quat) / dot(quat, quat); }

Quaternion lerp(const Quaternion& start, const Quaternion& end, float t)
{
    return normalize(start * (1.0f - t) + end * t);
}

Quaternion slerp(const Quaternion& start, const Quaternion& end, float t)
{
    float cosTheta = dot(normalize(start), normalize(end));
    Quaternion end2 = end;
    if (cosTheta < 0.0f)
    {
        cosTheta = -cosTheta;
        end2 = -end;
    }

    // Use lerp when quaternions are near or identical
    const float EPSILON = 0.001f;
    if (cosTheta > 1.0f - EPSILON)
    {
        return lerp(start, end2, t);
    }

    float theta = acos(cosTheta);
    return (sin((1.0f - t) * theta) * start + sin(t * theta) * end) / sin(theta);
}

vec3 rotate(const vec3& vec, const Quaternion& quat)
{
    const Quaternion vecQuat(0.0f, vec);
    return (quat * vecQuat * inverse(quat)).axis;
}

} // namespace huedra