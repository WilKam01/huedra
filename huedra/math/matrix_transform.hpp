#pragma once

#include "core/log.hpp"
#include "math/matrix.hpp"
#include "math/matrix_core.hpp"
#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"
#include "math/vec_transform.hpp"

#include <limits>

namespace huedra::math {

// Translate

template <typename T>
constexpr Matrix<T, 3, 3> translate(const Matrix<T, 3, 3>& matrix, const Vec2<T>& vec)
{
    Matrix<T, 3, 3> mat = matrix;
    mat(0, 2) = vec.x;
    mat(1, 2) = vec.y;
    return mat;
}

template <typename T>
constexpr Matrix<T, 4, 4> translate(const Matrix<T, 4, 4>& matrix, const Vec3<T>& vec)
{
    Matrix<T, 4, 4> mat = matrix;
    mat(0, 3) = vec.x;
    mat(1, 3) = vec.y;
    mat(2, 3) = vec.z;
    return mat;
}

// Rotate

// 2x2

template <typename T>
constexpr Matrix<T, 2, 2> rotate(const Matrix<T, 2, 2>& matrix, T angle)
{
    Matrix<T, 2, 2> mat = matrix;
    T c = cos(angle);
    T s = sin(angle);

    mat(0, 0) = c;
    mat(0, 1) = -s;
    mat(1, 0) = s;
    mat(1, 1) = c;
    return mat;
}

// 3x3

template <typename T>
constexpr Matrix<T, 3, 3> rotateX(const Matrix<T, 3, 3>& matrix, T angle)
{
    Matrix<T, 3, 3> mat = matrix;
    T c = cos(angle);
    T s = sin(angle);

    mat(1, 1) = c;
    mat(1, 2) = -s;
    mat(2, 1) = s;
    mat(2, 2) = c;
    return mat;
}

template <typename T>
constexpr Matrix<T, 3, 3> rotateY(const Matrix<T, 3, 3>& matrix, T angle)
{
    Matrix<T, 3, 3> mat = matrix;
    T c = cos(angle);
    T s = sin(angle);

    mat(0, 0) = c;
    mat(0, 2) = s;
    mat(2, 0) = -s;
    mat(2, 2) = c;
    return mat;
}

template <typename T>
constexpr Matrix<T, 3, 3> rotateZ(const Matrix<T, 3, 3>& matrix, T angle)
{
    Matrix<T, 3, 3> mat = matrix;
    T c = cos(angle);
    T s = sin(angle);

    mat(0, 0) = c;
    mat(0, 1) = -s;
    mat(1, 0) = s;
    mat(1, 1) = c;
    return mat;
}

template <typename T>
constexpr Matrix<T, 3, 3> rotate(const Matrix<T, 3, 3>& matrix, T angle, const Vec3<T>& axis)
{
    Matrix<T, 3, 3> mat = matrix;
    T c = cos(angle);
    T s = sin(angle);
    T oneMinusC = 1 - c;
    Vec3<T> v = normalize(axis);

    mat(0, 0) = c + v.x * v.x * oneMinusC;
    mat(0, 1) = v.x * v.y * oneMinusC - v.z * s;
    mat(0, 2) = v.x * v.z * oneMinusC + v.y * s;
    mat(1, 0) = v.y * v.x * oneMinusC + v.z * s;
    mat(1, 1) = c + v.y * v.y * oneMinusC;
    mat(1, 2) = v.y * v.z * oneMinusC - v.x * s;
    mat(2, 0) = v.z * v.x * oneMinusC - v.y * s;
    mat(2, 1) = v.z * v.y * oneMinusC + v.x * s;
    mat(2, 2) = c + v.z * v.z * oneMinusC;
    return mat;
}

// 4x4

template <typename T>
constexpr Matrix<T, 4, 4> rotateX(const Matrix<T, 4, 4>& matrix, T angle)
{
    Matrix<T, 4, 4> mat = matrix;
    T c = cos(angle);
    T s = sin(angle);

    mat(1, 1) = c;
    mat(1, 2) = -s;
    mat(2, 1) = s;
    mat(2, 2) = c;
    return mat;
}

template <typename T>
constexpr Matrix<T, 4, 4> rotateY(const Matrix<T, 4, 4>& matrix, T angle)
{
    Matrix<T, 4, 4> mat = matrix;
    T c = cos(angle);
    T s = sin(angle);

    mat(0, 0) = c;
    mat(0, 2) = s;
    mat(2, 0) = -s;
    mat(2, 2) = c;
    return mat;
}

template <typename T>
constexpr Matrix<T, 4, 4> rotateZ(const Matrix<T, 4, 4>& matrix, T angle)
{
    Matrix<T, 4, 4> mat = matrix;
    T c = cos(angle);
    T s = sin(angle);

    mat(0, 0) = c;
    mat(0, 1) = -s;
    mat(1, 0) = s;
    mat(1, 1) = c;
    return mat;
}

template <typename T>
constexpr Matrix<T, 4, 4> rotate(const Matrix<T, 4, 4>& matrix, T angle, const Vec3<T>& axis)
{
    Matrix<T, 4, 4> mat = matrix;
    T c = cos(angle);
    T s = sin(angle);
    T oneMinusC = 1 - c;
    Vec3<T> v = normalize(axis);

    mat(0, 0) = c + v.x * v.x * oneMinusC;
    mat(0, 1) = v.x * v.y * oneMinusC - v.z * s;
    mat(0, 2) = v.x * v.z * oneMinusC + v.y * s;
    mat(1, 0) = v.y * v.x * oneMinusC + v.z * s;
    mat(1, 1) = c + v.y * v.y * oneMinusC;
    mat(1, 2) = v.y * v.z * oneMinusC - v.x * s;
    mat(2, 0) = v.z * v.x * oneMinusC - v.y * s;
    mat(2, 1) = v.z * v.y * oneMinusC + v.x * s;
    mat(2, 2) = c + v.z * v.z * oneMinusC;
    return mat;
}

// Scale

template <typename T>
constexpr Matrix<T, 2, 2> scale(const Matrix<T, 2, 2>& matrix, const Vec2<T>& vec)
{
    Matrix<T, 2, 2> mat = matrix;
    mat(0, 0) = vec.x;
    mat(1, 1) = vec.y;
    return mat;
}

template <typename T>
constexpr Matrix<T, 3, 3> scale(const Matrix<T, 3, 3>& matrix, const Vec2<T>& vec)
{
    Matrix<T, 3, 3> mat = matrix;
    mat(0, 0) = vec.x;
    mat(1, 1) = vec.y;
    return mat;
}

template <typename T>
constexpr Matrix<T, 3, 3> scale(const Matrix<T, 3, 3>& matrix, const Vec3<T>& vec)
{
    Matrix<T, 3, 3> mat = matrix;
    mat(0, 0) = vec.x;
    mat(1, 1) = vec.y;
    mat(2, 2) = vec.z;
    return mat;
}

template <typename T>
constexpr Matrix<T, 4, 4> scale(const Matrix<T, 4, 4>& matrix, const Vec3<T>& vec)
{
    Matrix<T, 4, 4> mat = matrix;
    mat(0, 0) = vec.x;
    mat(1, 1) = vec.y;
    mat(2, 2) = vec.z;
    return mat;
}

// Shear

// vec.x = x, vec.y = y
template <typename T>
constexpr Matrix<T, 2, 2> shear(const Matrix<T, 2, 2>& matrix, const Vec2<T>& vec)
{
    Matrix<T, 2, 2> mat = matrix;
    mat(0, 1) = vec.x;
    mat(1, 0) = vec.y;
    return mat;
}

// v0.x = xy, v0.y = xz
// v1.x = yx, v1.y = yz
// v2.x = zx, v2.y = zy
template <typename T>
constexpr Matrix<T, 3, 3> shear(const Matrix<T, 3, 3>& matrix, const Vec2<T>& v0, const Vec2<T>& v1, const Vec2<T>& v2)
{
    Matrix<T, 3, 3> mat = matrix;
    mat(0, 1) = v0.x;
    mat(0, 2) = v0.y;
    mat(1, 0) = v1.x;
    mat(1, 2) = v1.y;
    mat(2, 0) = v2.x;
    mat(2, 1) = v2.y;
    return mat;
}

// v0.x = xy, v0.y = xz
// v1.x = yx, v1.y = yz
// v2.x = zx, v2.y = zy
template <typename T>
constexpr Matrix<T, 4, 4> shear(const Matrix<T, 4, 4>& matrix, const Vec2<T>& v0, const Vec2<T>& v1, const Vec2<T>& v2)
{
    Matrix<T, 4, 4> mat = matrix;
    mat(0, 1) = v0.x;
    mat(0, 2) = v0.y;
    mat(1, 0) = v1.x;
    mat(1, 2) = v1.y;
    mat(2, 0) = v2.x;
    mat(2, 1) = v2.y;
    return mat;
}

// LookAt

template <typename T>
constexpr Matrix<T, 4, 4> lookAtRH(const Vec3<T>& eye, const Vec3<T>& target, const Vec3<T>& up)
{
    Vec3<T> z = normalize(target - eye);
    Vec3<T> x = normalize(cross(z, up));
    Vec3<T> y = cross(x, z);

    Matrix<T, 4, 4> matrix(static_cast<T>(1));
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

    Matrix<T, 4, 4> matrix(static_cast<T>(1));
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