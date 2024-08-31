#pragma once

#include "math/matrix.hpp"
#include "math/matrix_core.hpp"
#include "math/vec2.hpp"
#include "math/vec3.hpp"

namespace huedra {

// Z = zero, N = negative, RH = right hand, LH = left hand
// width.x = left, width.y = right
// height.x = bottom, height.y = top
// zPlanes.x = near, zPlanes = far

// Ortho

// [0, 1]
template <typename T>
constexpr Matrix<T, 4, 4> orthoZRH(const Vec2<T>& width, const Vec2<T>& height, const Vec2<T>& zPlanes)
{
    Matrix<T, 4, 4> matrix(static_cast<T>(1));
    matrix(0, 0) = 2 / (width.y - width.x);
    matrix(1, 1) = 2 / (height.y - height.x);
    matrix(2, 2) = -1 / (zPlanes.y - zPlanes.x);
    matrix(0, 3) = -(width.y + width.x) / (width.y - width.x);
    matrix(1, 3) = -(height.y + height.x) / (height.y - height.x);
    matrix(2, 3) = -(zPlanes.x) / (zPlanes.y - zPlanes.x);
    return matrix;
}

// [0, 1]
template <typename T>
constexpr Matrix<T, 4, 4> orthoZLH(const Vec2<T>& width, const Vec2<T>& height, const Vec2<T>& zPlanes)
{
    Matrix<T, 4, 4> matrix(static_cast<T>(1));
    matrix(0, 0) = 2 / (width.y - width.x);
    matrix(1, 1) = 2 / (height.y - height.x);
    matrix(2, 2) = 1 / (zPlanes.y - zPlanes.x);
    matrix(0, 3) = -(width.y + width.x) / (width.y - width.x);
    matrix(1, 3) = -(height.y + height.x) / (height.y - height.x);
    matrix(2, 3) = -(zPlanes.x) / (zPlanes.y - zPlanes.x);
    return matrix;
}

// [-1, 1]
template <typename T>
constexpr Matrix<T, 4, 4> orthoNRH(const Vec2<T>& width, const Vec2<T>& height, const Vec2<T>& zPlanes)
{
    Matrix<T, 4, 4> matrix(static_cast<T>(1));
    matrix(0, 0) = 2 / (width.y - width.x);
    matrix(1, 1) = 2 / (height.y - height.x);
    matrix(2, 2) = -1 / (zPlanes.y - zPlanes.x);
    matrix(0, 3) = -(width.y + width.x) / (width.y - width.x);
    matrix(1, 3) = -(height.y + height.x) / (height.y - height.x);
    matrix(2, 3) = -(zPlanes.y + zPlanes.x) / (zPlanes.y - zPlanes.x);
    return matrix;
}

// [-1, 1]
template <typename T>
constexpr Matrix<T, 4, 4> orthoNLH(const Vec2<T>& width, const Vec2<T>& height, const Vec2<T>& zPlanes)
{
    Matrix<T, 4, 4> matrix(static_cast<T>(1));
    matrix(0, 0) = 2 / (width.y - width.x);
    matrix(1, 1) = 2 / (height.y - height.x);
    matrix(2, 2) = 1 / (zPlanes.y - zPlanes.x);
    matrix(0, 3) = -(width.y + width.x) / (width.y - width.x);
    matrix(1, 3) = -(height.y + height.x) / (height.y - height.x);
    matrix(2, 3) = -(zPlanes.y + zPlanes.x) / (zPlanes.y - zPlanes.x);
    return matrix;
}

// [0, 1]
template <typename T>
constexpr Matrix<T, 4, 4> orthoZ(const Vec2<T>& width, const Vec2<T>& height, const Vec2<T>& zPlanes)
{
    // TODO: Add choice of LH vs RH
    return orthoZRH(width, height, zPlanes);
}

// [-1, 1]
template <typename T>
constexpr Matrix<T, 4, 4> orthoN(const Vec2<T>& width, const Vec2<T>& height, const Vec2<T>& zPlanes)
{
    // TODO: Add choice of LH vs RH
    return orthoNRH(width, height, zPlanes);
}

template <typename T>
constexpr Matrix<T, 4, 4> orthoRH(const Vec2<T>& width, const Vec2<T>& height, const Vec2<T>& zPlanes)
{
    // TODO: Add choice of [0, 1] vs [-1, 1]
    return orthoZRH(width, height, zPlanes);
}

template <typename T>
constexpr Matrix<T, 4, 4> orthoLH(const Vec2<T>& width, const Vec2<T>& height, const Vec2<T>& zPlanes)
{
    // TODO: Add choice of [0, 1] vs [-1, 1]
    return orthoZLH(width, height, zPlanes);
}

template <typename T>
constexpr Matrix<T, 4, 4> ortho(const Vec2<T>& width, const Vec2<T>& height, const Vec2<T>& zPlanes)
{
    // TODO: Add choice of RH, LH, [0, 1] and [-1, 1]
    return orthoZRH(width, height, zPlanes);
}

// Perspective

// [0, 1]
template <typename T>
constexpr Matrix<T, 4, 4> perspectiveZRH(T fov, T aspectRatio, const Vec2<T>& zPlanes)
{
    T tanVert = tan(fov / static_cast<T>(2));

    Matrix<T, 4, 4> matrix;
    matrix(0, 0) = static_cast<T>(1) / (aspectRatio * tanVert);
    matrix(1, 1) = static_cast<T>(1) / tanVert;
    matrix(2, 2) = (zPlanes.y) / (zPlanes.x - zPlanes.y);
    matrix(3, 2) = -static_cast<T>(1);
    matrix(2, 3) = -(zPlanes.y * zPlanes.x) / (zPlanes.y - zPlanes.x);

    return matrix;
}

// [-1, 1]
template <typename T>
constexpr Matrix<T, 4, 4> perspectiveNRH(T fov, T aspectRatio, const Vec2<T>& zPlanes)
{
    T tanVert = tan(fov / static_cast<T>(2));

    Matrix<T, 4, 4> matrix;
    matrix(0, 0) = static_cast<T>(1) / (aspectRatio * tanVert);
    matrix(1, 1) = static_cast<T>(1) / tanVert;
    matrix(2, 2) = -(zPlanes.y + zPlanes.x) / (zPlanes.y - zPlanes.x);
    matrix(3, 2) = -static_cast<T>(1);
    matrix(2, 3) = -(2 * zPlanes.y * zPlanes.x) / (zPlanes.y - zPlanes.x);

    return matrix;
}

// [0, 1]
template <typename T>
constexpr Matrix<T, 4, 4> perspectiveZLH(T fov, T aspectRatio, const Vec2<T>& zPlanes)
{
    T tanVert = tan(fov / static_cast<T>(2));

    Matrix<T, 4, 4> matrix;
    matrix(0, 0) = static_cast<T>(1) / (aspectRatio * tanVert);
    matrix(1, 1) = static_cast<T>(1) / tanVert;
    matrix(2, 2) = (zPlanes.y) / (zPlanes.y - zPlanes.x);
    matrix(3, 2) = static_cast<T>(1);
    matrix(2, 3) = -(zPlanes.y * zPlanes.x) / (zPlanes.y - zPlanes.x);

    return matrix;
}

// [-1, 1]
template <typename T>
constexpr Matrix<T, 4, 4> perspectiveNLH(T fov, T aspectRatio, const Vec2<T>& zPlanes)
{
    T tanVert = tan(fov / static_cast<T>(2));

    Matrix<T, 4, 4> matrix;
    matrix(0, 0) = static_cast<T>(1) / (aspectRatio * tanVert);
    matrix(1, 1) = static_cast<T>(1) / tanVert;
    matrix(2, 2) = (zPlanes.y + zPlanes.x) / (zPlanes.y - zPlanes.x);
    matrix(3, 2) = static_cast<T>(1);
    matrix(2, 3) = -(2 * zPlanes.y * zPlanes.x) / (zPlanes.y - zPlanes.x);

    return matrix;
}

// [0, 1]
template <typename T>
constexpr Matrix<T, 4, 4> perspectiveZ(T fov, T aspectRatio, const Vec2<T>& zPlanes)
{
    // TODO: Add choice of LH vs RH
    return perspectiveZRH(fov, aspectRatio, zPlanes);
}

// [-1, 1]
template <typename T>
constexpr Matrix<T, 4, 4> perspectiveN(T fov, T aspectRatio, const Vec2<T>& zPlanes)
{
    // TODO: Add choice of LH vs RH
    return perspectiveNRH(fov, aspectRatio, zPlanes);
}

template <typename T>
constexpr Matrix<T, 4, 4> perspectiveRH(T fov, T aspectRatio, const Vec2<T>& zPlanes)
{
    // TODO: Add choice of [0, 1] vs [-1, 1]
    return perspectiveZRH(fov, aspectRatio, zPlanes);
}

template <typename T>
constexpr Matrix<T, 4, 4> perspectiveLH(T fov, T aspectRatio, const Vec2<T>& zPlanes)
{
    // TODO: Add choice of [0, 1] vs [-1, 1]
    return perspectiveZLH(fov, aspectRatio, zPlanes);
}

template <typename T>
constexpr Matrix<T, 4, 4> perspective(T fov, T aspectRatio, const Vec2<T>& zPlanes)
{
    // TODO: Add choice of RH, LH, [0, 1] and [-1, 1]
    return perspectiveZRH(fov, aspectRatio, zPlanes);
}

} // namespace huedra