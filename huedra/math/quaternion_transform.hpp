#pragma once

#include "math/quaternion.hpp"

namespace huedra::math {

float inline dot(const Quaternion& lhs, const Quaternion& rhs)
{
    return (lhs.scalar * rhs.scalar) + dot(lhs.axis, rhs.axis);
}

float inline length(const Quaternion& quat) { return sqrt(dot(quat, quat)); }

Quaternion inline normalize(const Quaternion& quat) { return quat / length(quat); }

Quaternion inline conjugate(const Quaternion& quat) { return {quat.scalar, -quat.axis}; }

Quaternion inline inverse(const Quaternion& quat) { return conjugate(quat) / dot(quat, quat); }

Quaternion inline lerp(const Quaternion& start, const Quaternion& end, float t)
{
    return normalize(start * (1.0f - t) + end * t);
}

Quaternion inline slerp(const Quaternion& start, const Quaternion& end, float t)
{
    float cosTheta = dot(normalize(start), normalize(end));
    Quaternion end2 = end;
    if (cosTheta < 0.0f)
    {
        cosTheta = -cosTheta;
        end2 = -end;
    }

    // Use lerp when quaternions are near or identical
    if (cosTheta > 1.0f - std::numeric_limits<float>::epsilon())
    {
        return lerp(start, end2, t);
    }

    float theta = acos(cosTheta);
    return (sin((1.0f - t) * theta) * start + sin(t * theta) * end) / sin(theta);
}

vec3 inline rotate(const vec3& vec, const Quaternion& quat)
{
    const Quaternion vecQuat(0.0f, vec);
    return (quat * vecQuat * inverse(quat)).axis;
}

} // namespace huedra::math