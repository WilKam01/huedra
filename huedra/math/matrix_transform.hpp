#pragma once

#include "core/log.hpp"
#include "math/matrix.hpp"
#include "math/matrix_core.hpp"
#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"
#include "math/vec_transform.hpp"

#include <limits>

namespace huedra {

// Translate

template <typename T>
constexpr void translate(Matrix<T, 3, 3>& matrix, const Vec2<T>& vec)
{
    matrix(0, 2) = vec.x;
    matrix(1, 2) = vec.y;
}

template <typename T>
constexpr Matrix<T, 3, 3> translate(const Vec2<T>& vec)
{
    Matrix<T, 3, 3> matrix = identity<T, 3>();
    translate(matrix, vec);
    return matrix;
}

template <typename T>
constexpr void translate(Matrix<T, 4, 4>& matrix, const Vec3<T>& vec)
{
    matrix(0, 3) = vec.x;
    matrix(1, 3) = vec.y;
    matrix(2, 3) = vec.z;
}

template <typename T>
constexpr Matrix<T, 4, 4> translate(const Vec3<T>& vec)
{
    Matrix<T, 4, 4> matrix = identity<T, 4>();
    translate(matrix, vec);
    return matrix;
}

// Rotate

// 2x2

template <typename T>
constexpr void rotate(Matrix<T, 2, 2>& matrix, T angle)
{
    T c = cos(angle);
    T s = sin(angle);

    matrix(0, 0) = c;
    matrix(0, 1) = -s;
    matrix(1, 0) = s;
    matrix(1, 1) = c;
}

template <typename T>
constexpr Matrix<T, 2, 2> rotate(T angle)
{
    Matrix<T, 2, 2> matrix = identity<T, 2>();
    rotate(matrix, angle);
    return matrix;
}

// 3x3

template <typename T>
constexpr void rotateX(Matrix<T, 3, 3>& matrix, T angle)
{
    T c = cos(angle);
    T s = sin(angle);

    matrix(1, 1) = c;
    matrix(1, 2) = -s;
    matrix(2, 1) = s;
    matrix(2, 2) = c;
}

template <typename T>
constexpr Matrix<T, 3, 3> rotateX(T angle)
{
    Matrix<T, 3, 3> matrix = identity<T, 3>();
    rotateX(matrix, angle);
    return matrix;
}

template <typename T>
constexpr void rotateY(Matrix<T, 3, 3>& matrix, T angle)
{
    T c = cos(angle);
    T s = sin(angle);

    matrix(0, 0) = c;
    matrix(0, 2) = s;
    matrix(2, 0) = -s;
    matrix(2, 2) = c;
}

template <typename T>
constexpr Matrix<T, 3, 3> rotateY(T angle)
{
    Matrix<T, 3, 3> matrix = identity<T, 3>();
    rotateY(matrix, angle);
    return matrix;
}

template <typename T>
constexpr void rotateZ(Matrix<T, 3, 3>& matrix, T angle)
{
    T c = cos(angle);
    T s = sin(angle);

    matrix(0, 0) = c;
    matrix(0, 1) = -s;
    matrix(1, 0) = s;
    matrix(1, 1) = c;
}

template <typename T>
constexpr Matrix<T, 3, 3> rotateZ(T angle)
{
    Matrix<T, 3, 3> matrix = identity<T, 3>();
    rotateZ(matrix, angle);
    return matrix;
}

template <typename T>
constexpr void rotate(Matrix<T, 3, 3>& matrix, T angle, const Vec3<T>& axis)
{
    T c = cos(angle);
    T s = sin(angle);
    T oneMinusC = 1 - c;
    Vec3<T> v = normalize(axis);

    matrix(0, 0) = c + v.x * v.x * oneMinusC;
    matrix(0, 1) = v.x * v.y * oneMinusC - v.z * s;
    matrix(0, 2) = v.x * v.z * oneMinusC + v.y * s;
    matrix(1, 0) = v.y * v.x * oneMinusC + v.z * s;
    matrix(1, 1) = c + v.y * v.y * oneMinusC;
    matrix(1, 2) = v.y * v.z * oneMinusC - v.x * s;
    matrix(2, 0) = v.z * v.x * oneMinusC - v.y * s;
    matrix(2, 1) = v.z * v.y * oneMinusC + v.x * s;
    matrix(2, 2) = c + v.z * v.z * oneMinusC;
}

template <typename T>
constexpr Matrix<T, 3, 3> rotate(T angle, const Vec3<T>& axis)
{
    Matrix<T, 3, 3> matrix = identity<T, 3>();
    rotate(matrix, angle, axis);
    return matrix;
}

// 4x4

template <typename T>
constexpr void rotateX(Matrix<T, 4, 4>& matrix, T angle)
{
    T c = cos(angle);
    T s = sin(angle);

    matrix(1, 1) = c;
    matrix(1, 2) = -s;
    matrix(2, 1) = s;
    matrix(2, 2) = c;
}

template <typename T>
constexpr Matrix<T, 4, 4> rotateX(T angle)
{
    Matrix<T, 4, 4> matrix = identity<T, 4>();
    rotateX(matrix);
    return matrix;
}

template <typename T>
constexpr void rotateY(Matrix<T, 4, 4>& matrix, T angle)
{
    T c = cos(angle);
    T s = sin(angle);

    matrix(0, 0) = c;
    matrix(0, 2) = s;
    matrix(2, 0) = -s;
    matrix(2, 2) = c;
}

template <typename T>
constexpr Matrix<T, 4, 4> rotateY(T angle)
{
    Matrix<T, 4, 4> matrix = identity<T, 4>();
    rotateY(matrix);
    return matrix;
}

template <typename T>
constexpr void rotateZ(Matrix<T, 4, 4>& matrix, T angle)
{
    T c = cos(angle);
    T s = sin(angle);

    matrix(0, 0) = c;
    matrix(0, 1) = -s;
    matrix(1, 0) = s;
    matrix(1, 1) = c;
}

template <typename T>
constexpr Matrix<T, 4, 4> rotateZ(T angle)
{
    Matrix<T, 4, 4> matrix = identity<T, 4>();
    rotateZ(matrix);
    return matrix;
}

template <typename T>
constexpr void rotate(Matrix<T, 4, 4>& matrix, T angle, const Vec3<T>& axis)
{
    T c = cos(angle);
    T s = sin(angle);
    T oneMinusC = 1 - c;
    Vec3<T> v = normalize(axis);

    matrix(0, 0) = c + v.x * v.x * oneMinusC;
    matrix(0, 1) = v.x * v.y * oneMinusC - v.z * s;
    matrix(0, 2) = v.x * v.z * oneMinusC + v.y * s;
    matrix(1, 0) = v.y * v.x * oneMinusC + v.z * s;
    matrix(1, 1) = c + v.y * v.y * oneMinusC;
    matrix(1, 2) = v.y * v.z * oneMinusC - v.x * s;
    matrix(2, 0) = v.z * v.x * oneMinusC - v.y * s;
    matrix(2, 1) = v.z * v.y * oneMinusC + v.x * s;
    matrix(2, 2) = c + v.z * v.z * oneMinusC;
}

template <typename T>
constexpr Matrix<T, 4, 4> rotate(T angle, const Vec3<T>& axis)
{
    Matrix<T, 4, 4> matrix = identity<T, 4>();
    rotate(matrix, angle, axis);
    return matrix;
}

// Scale

template <typename T>
constexpr void scale(Matrix<T, 2, 2>& matrix, const Vec2<T>& vec)
{
    matrix(0, 0) = vec.x;
    matrix(1, 1) = vec.y;
}

template <typename T>
constexpr Matrix<T, 2, 2> scale(const Vec2<T>& vec)
{
    Matrix<T, 2, 2> matrix = identity<T, 2>();
    scale(matrix, vec);
    return matrix;
}

template <typename T>
constexpr void scale(Matrix<T, 3, 3>& matrix, const Vec2<T>& vec)
{
    matrix(0, 0) = vec.x;
    matrix(1, 1) = vec.y;
}

template <typename T>
constexpr Matrix<T, 3, 3> scale(const Vec2<T>& vec)
{
    Matrix<T, 3, 3> matrix = identity<T, 3>();
    scale(matrix, vec);
    return matrix;
}

template <typename T>
constexpr void scale(Matrix<T, 3, 3>& matrix, const Vec3<T>& vec)
{
    matrix(0, 0) = vec.x;
    matrix(1, 1) = vec.y;
    matrix(2, 2) = vec.z;
}

template <typename T>
constexpr Matrix<T, 3, 3> scale(const Vec3<T>& vec)
{
    Matrix<T, 3, 3> matrix = identity<T, 3>();
    scale(matrix, vec);
    return matrix;
}

template <typename T>
constexpr void scale(Matrix<T, 4, 4>& matrix, const Vec3<T>& vec)
{
    matrix(0, 0) = vec.x;
    matrix(1, 1) = vec.y;
    matrix(2, 2) = vec.z;
}

template <typename T>
constexpr Matrix<T, 4, 4> scale(const Vec3<T>& vec)
{
    Matrix<T, 4, 4> matrix = identity<T, 4>();
    scale(matrix, vec);
    return matrix;
}

// Shear

// vec.x = x, vec.y = y
template <typename T>
constexpr void shear(Matrix<T, 2, 2>& matrix, const Vec2<T>& vec)
{
    matrix(0, 1) = vec.x;
    matrix(1, 0) = vec.y;
}

// vec.x = x, vec.y = y
template <typename T>
constexpr Matrix<T, 2, 2> shear(const Vec2<T>& vec)
{
    Matrix<T, 2, 2> matrix = identity<T, 2>();
    shear(matrix, vec);
    return matrix;
}

// v0.x = xy, v0.y = xz
// v1.x = yx, v1.y = yz
// v2.x = zx, v2.y = zy
template <typename T>
constexpr void shear(Matrix<T, 3, 3>& matrix, const Vec2<T>& v0, const Vec2<T>& v1, const Vec2<T>& v2)
{
    matrix(0, 1) = v0.x;
    matrix(0, 2) = v0.y;
    matrix(1, 0) = v1.x;
    matrix(1, 2) = v1.y;
    matrix(2, 0) = v2.x;
    matrix(2, 1) = v2.y;
}

// v0.x = xy, v0.y = xz
// v1.x = yx, v1.y = yz
// v2.x = zx, v2.y = zy
template <typename T>
constexpr Matrix<T, 3, 3> shear(const Vec2<T>& v0, const Vec2<T>& v1, const Vec2<T>& v2)
{
    Matrix<T, 3, 3> matrix = identity<T, 3>();
    shear(matrix, v0, v1, v2);
    return matrix;
}

// v0.x = xy, v0.y = xz
// v1.x = yx, v1.y = yz
// v2.x = zx, v2.y = zy
template <typename T>
constexpr void shear(Matrix<T, 4, 4>& matrix, const Vec2<T>& v0, const Vec2<T>& v1, const Vec2<T>& v2)
{
    matrix(0, 1) = v0.x;
    matrix(0, 2) = v0.y;
    matrix(1, 0) = v1.x;
    matrix(1, 2) = v1.y;
    matrix(2, 0) = v2.x;
    matrix(2, 1) = v2.y;
}

// v0.x = xy, v0.y = xz
// v1.x = yx, v1.y = yz
// v2.x = zx, v2.y = zy
template <typename T>
constexpr Matrix<T, 4, 4> shear(const Vec2<T>& v0, const Vec2<T>& v1, const Vec2<T>& v2)
{
    Matrix<T, 4, 4> matrix = identity<T, 4>();
    shear(matrix, v0, v1, v2);
    return matrix;
}

// LookAt

template <typename T>
constexpr Matrix<T, 4, 4> lookAtRH(const Vec3<T>& eye, const Vec3<T>& target, const Vec3<T>& up)
{
    Vec3<T> z = normalize(target - eye);
    Vec3<T> x = normalize(cross(z, up));
    Vec3<T> y = cross(x, z);

    Matrix<T, 4, 4> matrix = identity<T, 4>();
    matrix(0, 0) = x.x;
    matrix(0, 1) = x.y;
    matrix(0, 2) = x.z;
    matrix(0, 3) = -dot(x, eye);

    matrix(1, 0) = y.x;
    matrix(1, 1) = y.y;
    matrix(1, 2) = y.z;
    matrix(1, 3) = -dot(y, eye);

    matrix(2, 0) = -z.x;
    matrix(2, 1) = -z.y;
    matrix(2, 2) = -z.z;
    matrix(2, 3) = dot(z, eye);

    return matrix;
}

template <typename T>
constexpr Matrix<T, 4, 4> lookAtLH(const Vec3<T>& eye, const Vec3<T>& target, const Vec3<T>& up)
{
    Vec3<T> z = normalize(target - eye);
    Vec3<T> x = normalize(cross(up, z));
    Vec3<T> y = cross(z, x);

    Matrix<T, 4, 4> matrix = identity<T, 4>();
    matrix(0, 0) = x.x;
    matrix(0, 1) = x.y;
    matrix(0, 2) = x.z;
    matrix(1, 0) = y.x;
    matrix(1, 1) = y.y;
    matrix(1, 2) = y.z;
    matrix(2, 0) = -z.x;
    matrix(2, 1) = -z.y;
    matrix(2, 2) = -z.z;
    matrix(0, 3) = -dot(x, eye);
    matrix(1, 3) = -dot(y, eye);
    matrix(2, 3) = dot(z, eye);
    return matrix;
}

template <typename T>
constexpr Matrix<T, 4, 4> lookAt(const Vec3<T>& eye, const Vec3<T>& target, const Vec3<T>& up)
{
    // TODO: Add choice of LH vs RH
    return lookAtRH(eye, target, up);
}

// Look To

template <typename T>
constexpr Matrix<T, 4, 4> lookToRH(const Vec3<T>& eye, const Vec3<T>& direction, const Vec3<T>& up)
{
    Vec3<T> target = eye + direction;
    return lookAtRH(eye, target, up);
}

template <typename T>
constexpr Matrix<T, 4, 4> lookToLH(const Vec3<T>& eye, const Vec3<T>& direction, const Vec3<T>& up)
{
    Vec3<T> target = eye + direction;
    return lookAtLH(eye, target, up);
}

template <typename T>
constexpr Matrix<T, 4, 4> lookTo(const Vec3<T>& eye, const Vec3<T>& direction, const Vec3<T>& up)
{
    // TODO: Add choice of LH vs RH
    Vec3<T> target = eye + direction;
    return lookAtRH(eye, target, up);
}

} // namespace huedra