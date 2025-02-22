#pragma once

#include "core/types.hpp"
#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"

namespace huedra {

template <typename T, u64 R, u64 C>
class Matrix
{
public:
    Matrix() = default;
    explicit Matrix(T scalar)
    {
        static_assert(R == C, "Scalar constructor only works on square matrices");
        for (u64 i = 0; i < C; ++i)
        {
            m_elements[i][i] = scalar;
        }
    }
    explicit Matrix(std::array<std::array<T, R>, C> values) : m_elements(values) {}
    explicit Matrix(std::array<T, R * C> values)
    {
        for (u64 i = 0; i < C; ++i)
        {
            for (u64 j = 0; j < R; ++j)
            {
                m_elements[i][j] = values[(i * R) + j];
            }
        }
    }

    T operator[](u64 index) const { return m_elements[index / C][index % C]; }
    T& operator[](u64 index) { return m_elements[index / C][index % C]; }

    T operator()(u64 row, u64 column) const { return m_elements[column][row]; }
    T& operator()(u64 row, u64 column) { return m_elements[column][row]; }

    constexpr Matrix<T, R, C> operator+(T scalar)
    {
        Matrix<T, R, C> matrix(m_elements);
        for (u64 i = 0; i < C; ++i)
        {
            for (u64 j = 0; j < R; ++j)
            {
                matrix.m_elements[i][j] += scalar;
            }
        }
        return matrix;
    }

    constexpr Matrix<T, R, C> operator-(T scalar)
    {
        Matrix<T, R, C> matrix(m_elements);
        for (u64 i = 0; i < C; ++i)
        {
            for (u64 j = 0; j < R; ++j)
            {
                matrix.m_elements[i][j] -= scalar;
            }
        }
        return matrix;
    }

    constexpr Matrix<T, R, C> operator*(T scalar)
    {
        Matrix<T, R, C> matrix(m_elements);
        for (u64 i = 0; i < C; ++i)
        {
            for (u64 j = 0; j < R; ++j)
            {
                matrix.m_elements[i][j] *= scalar;
            }
        }
        return matrix;
    }

    constexpr Matrix<T, R, C> operator/(T scalar)
    {
        Matrix<T, R, C> matrix(m_elements);
        for (u64 i = 0; i < C; ++i)
        {
            for (u64 j = 0; j < R; ++j)
            {
                matrix.m_elements[i][j] /= scalar;
            }
        }
        return matrix;
    }

    template <u64 M>
    constexpr Matrix<T, R, M> operator*(const Matrix<T, C, M>& matrix)
    {
        Matrix<T, R, M> ret;
        for (u64 i = 0; i < M; ++i)
        {
            for (u64 j = 0; j < R; ++j)
            {
                for (u64 k = 0; k < C; ++k)
                {
                    ret.m_elements[i][j] += m_elements[k][j] * matrix.m_elements[i][k];
                }
            }
        }

        return ret;
    }

    constexpr Matrix<T, R, C>& operator*=(const Matrix<T, R, C>& matrix)
    {
        *this = *this * matrix;
        return *this;
    }

    constexpr Vec2<T> operator*(const Vec2<T>& vec)
    {
        static_assert(R == 2 && C == 2, "Could not multiply Vec2 with a non 2x2 matrix");
        Vec2<T> ret;
        for (u64 i = 0; i < 2; ++i)
        {
            ret.x += m_elements[i][0] * vec[i];
            ret.y += m_elements[i][1] * vec[i];
        }
        return ret;
    }

    constexpr Vec3<T> operator*(const Vec3<T>& vec)
    {
        static_assert(R == 3 && C == 3, "Could not multiply Vec3 with a non 3x3 matrix");
        Vec3<T> ret;
        for (u64 i = 0; i < 3; ++i)
        {
            ret.x += m_elements[i][0] * vec[i];
            ret.y += m_elements[i][1] * vec[i];
            ret.z += m_elements[i][2] * vec[i];
        }
        return ret;
    }

    constexpr Vec4<T> operator*(const Vec4<T>& vec)
    {
        static_assert(R == 4 && C == 4, "Could not multiply Vec4 with a non 4x4 matrix");
        Vec4<T> ret;
        for (u64 i = 0; i < 4; ++i)
        {
            ret.x += m_elements[i][0] * vec[i];
            ret.y += m_elements[i][1] * vec[i];
            ret.z += m_elements[i][2] * vec[i];
            ret.w += m_elements[i][3] * vec[i];
        }
        return ret;
    }

private:
    std::array<std::array<T, R>, C> m_elements{static_cast<T>(0)};
};

using matrix2x2 = Matrix<float, 2, 2>;
using matrix3x3 = Matrix<float, 3, 3>;
using matrix4x4 = Matrix<float, 4, 4>;
using matrix2 = Matrix<float, 2, 2>;
using matrix3 = Matrix<float, 3, 3>;
using matrix4 = Matrix<float, 4, 4>;

using dmatrix2x2 = Matrix<double, 2, 2>;
using dmatrix3x3 = Matrix<double, 3, 3>;
using dmatrix4x4 = Matrix<double, 4, 4>;
using dmatrix2 = Matrix<double, 2, 2>;
using dmatrix3 = Matrix<double, 3, 3>;
using dmatrix4 = Matrix<double, 4, 4>;

} // namespace huedra