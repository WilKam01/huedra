#pragma once

#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"
#include <cmath>

namespace huedra {

// Dot Product

template <typename T>
T dot(const Vec2<T>& lhs, const Vec2<T>& rhs)
{
    return lhs.x * rhs.x + lhs.y * rhs.y;
}

template <typename T>
T dot(const Vec3<T>& lhs, const Vec3<T>& rhs)
{
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

template <typename T>
T dot(const Vec4<T>& lhs, const Vec4<T>& rhs)
{
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
}

// Cross Product

template <typename T>
T cross(const Vec2<T>& lhs, const Vec2<T>& rhs)
{
    return lhs.x * rhs.y - lhs.y * rhs.x;
}

template <typename T>
Vec3<T> cross(const Vec3<T>& lhs, const Vec3<T>& rhs)
{
    return Vec3(lhs.y * rhs.z - lhs.z * rhs.y, lhs.z * rhs.x - lhs.x * rhs.z, lhs.x * rhs.y - lhs.y * rhs.x);
}

// Squared Length

template <typename T>
T squaredLength(const Vec2<T>& vec)
{
    return dot(vec, vec);
}

template <typename T>
T squaredLength(const Vec3<T>& vec)
{
    return dot(vec, vec);
}

template <typename T>
T squaredLength(const Vec4<T>& vec)
{
    return dot(vec, vec);
}

// Length

template <typename T>
T length(const Vec2<T>& vec)
{
    return sqrt(dot(vec, vec));
}

template <typename T>
T length(const Vec3<T>& vec)
{
    return sqrt(dot(vec, vec));
}

template <typename T>
T length(const Vec4<T>& vec)
{
    return sqrt(dot(vec, vec));
}

// Normalize

template <typename T>
Vec2<T> normalize(const Vec2<T>& vec)
{
    return vec / length(vec);
}

template <typename T>
Vec3<T> normalize(const Vec3<T>& vec)
{
    return vec / length(vec);
}

template <typename T>
Vec4<T> normalize(const Vec4<T>& vec)
{
    return vec / length(vec);
}

// Distance

template <typename T>
T distance(const Vec2<T>& lhs, const Vec2<T>& rhs)
{
    return length(lhs - rhs);
}

template <typename T>
T distance(const Vec3<T>& lhs, const Vec3<T>& rhs)
{
    return length(lhs - rhs);
}

template <typename T>
T distance(const Vec4<T>& lhs, const Vec4<T>& rhs)
{
    return length(lhs - rhs);
}

// Project

template <typename T>
Vec2<T> project(const Vec2<T>& vec, const Vec2<T>& other)
{
    return other * (dot(vec, other) / dot(other, other));
}

template <typename T>
Vec3<T> project(const Vec3<T>& vec, const Vec3<T>& other)
{
    return other * (dot(vec, other) / dot(other, other));
}

template <typename T>
Vec4<T> project(const Vec4<T>& vec, const Vec4<T>& other)
{
    return other * (dot(vec, other) / dot(other, other));
}

// Reflect

template <typename T>
Vec2<T> reflect(const Vec2<T>& vec, const Vec2<T>& normal)
{
    return vec - (2 * dot(normal, normal)) * normal;
}

template <typename T>
Vec3<T> reflect(const Vec3<T>& vec, const Vec3<T>& normal)
{
    return vec - (2 * dot(normal, normal)) * normal;
}

template <typename T>
Vec4<T> reflect(const Vec4<T>& vec, const Vec4<T>& normal)
{
    return vec - (2 * dot(normal, normal)) * normal;
}

// Angle (radians)

template <typename T>
T angle(const Vec2<T>& lhs, const Vec2<T>& rhs)
{
    return acos(dot(lhs, rhs) / (length(lhs) * length(rhs)));
}

template <typename T>
T angle(const Vec3<T>& lhs, const Vec3<T>& rhs)
{
    return acos(dot(lhs, rhs) / (length(lhs) * length(rhs)));
}

template <typename T>
T angle(const Vec4<T>& lhs, const Vec4<T>& rhs)
{
    return acos(dot(lhs, rhs) / (length(lhs) * length(rhs)));
}

// Lerp

template <typename T>
Vec2<T> lerp(const Vec2<T>& start, const Vec2<T>& end, float t)
{
    return start * (1.0f - t) + end * t;
}

template <typename T>
Vec3<T> lerp(const Vec3<T>& start, const Vec3<T>& end, float t)
{
    return start * (1.0f - t) + end * t;
}

template <typename T>
Vec4<T> lerp(const Vec4<T>& start, const Vec4<T>& end, float t)
{
    return start * (1.0f - t) + end * t;
}

// Slerp

template <typename T>
Vec2<T> slerp(const Vec2<T>& start, const Vec2<T>& end, float t)
{
    T cosTheta = dot(normalize(start), normalize(end));
    T theta = acos(cosTheta);
    T sinTheta = sin(theta);

    return (sin(std::abs((1.0f - t) * theta)) / sinTheta) * start + (sin(std::abs(t * theta)) / sinTheta) * end;
}

template <typename T>
Vec3<T> slerp(const Vec3<T>& start, const Vec3<T>& end, float t)
{
    T cosTheta = dot(normalize(start), normalize(end));
    T theta = acos(cosTheta);
    T sinTheta = sin(theta);

    return (sin(std::abs((1.0f - t) * theta)) / sinTheta) * start + (sin(std::abs(t * theta)) / sinTheta) * end;
}

template <typename T>
Vec4<T> slerp(const Vec4<T>& start, const Vec4<T>& end, float t)
{
    T cosTheta = dot(normalize(start), normalize(end));
    T theta = acos(cosTheta);
    T sinTheta = sin(theta);

    return (sin(std::abs((1.0f - t) * theta)) / sinTheta) * start + (sin(std::abs(t * theta)) / sinTheta) * end;
}

} // namespace huedra
