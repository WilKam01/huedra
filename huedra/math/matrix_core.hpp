#pragma once

namespace huedra {

// Determinant

template <typename T>
constexpr T determinant(const Matrix<T, 2, 2>& matrix)
{
    return matrix(0, 0) * matrix(1, 1) - matrix(0, 1) * matrix(1, 0);
}

template <typename T>
constexpr T determinant(const Matrix<T, 3, 3>& matrix)
{
    Matrix<T, 2, 2> a({{matrix(1, 1), matrix(2, 1), matrix(1, 2), matrix(2, 2)}});
    Matrix<T, 2, 2> b({{matrix(1, 0), matrix(2, 0), matrix(1, 2), matrix(2, 2)}});
    Matrix<T, 2, 2> c({{matrix(1, 0), matrix(2, 0), matrix(1, 1), matrix(2, 1)}});

    return matrix(0, 0) * determinant(a) - matrix(0, 1) * determinant(b) + matrix(0, 2) * determinant(c);
}

template <typename T>
constexpr T determinant(const Matrix<T, 4, 4>& matrix)
{
    Matrix<T, 3, 3> a({{matrix(1, 1), matrix(2, 1), matrix(3, 1), matrix(1, 2), matrix(2, 2), matrix(3, 2),
                        matrix(1, 3), matrix(2, 3), matrix(3, 3)}});
    Matrix<T, 3, 3> b({{matrix(1, 0), matrix(2, 0), matrix(3, 0), matrix(1, 2), matrix(2, 2), matrix(3, 2),
                        matrix(1, 3), matrix(2, 3), matrix(3, 3)}});
    Matrix<T, 3, 3> c({{matrix(1, 0), matrix(2, 0), matrix(3, 0), matrix(1, 1), matrix(2, 1), matrix(3, 1),
                        matrix(1, 3), matrix(2, 3), matrix(3, 3)}});
    Matrix<T, 3, 3> d({{matrix(1, 0), matrix(2, 0), matrix(3, 0), matrix(1, 1), matrix(2, 1), matrix(3, 1),
                        matrix(1, 2), matrix(2, 2), matrix(3, 2)}});

    return matrix(0, 0) * determinant(a) - matrix(0, 1) * determinant(b) + matrix(0, 2) * determinant(c) -
           matrix(0, 3) * determinant(d);
}

// Transpose

template <typename T, u64 R, u64 C>
constexpr Matrix<T, C, R> transpose(const Matrix<T, R, C>& matrix)
{
    Matrix<T, C, R> ret;
    for (u64 i = 0; i < R; ++i)
    {
        for (u64 j = 0; j < C; ++j)
        {
            ret(j, i) = matrix(i, j);
        }
    }

    return ret;
}

// Invert

template <typename T>
constexpr Matrix<T, 2, 2> invert(const Matrix<T, 2, 2>& matrix)
{
    T det = determinant(matrix);
    if (abs(det) < std::numeric_limits<T>::epsilon())
    {
        log(LogLevel::WARNING, "Could not invert 2x2 matrix, determinant = 0");
        return matrix;
    }

    return Matrix<T, 2, 2>({{matrix(1, 1), -matrix(1, 0), -matrix(0, 1), matrix(0, 0)}}) / det;
}

template <typename T>
constexpr Matrix<T, 3, 3> invert(const Matrix<T, 3, 3>& matrix)
{
    T det = determinant(matrix);
    if (abs(det) < std::numeric_limits<T>::epsilon())
    {
        log(LogLevel::WARNING, "Could not invert 3x3 matrix, determinant = 0");
        return matrix;
    }

    T a = matrix(0, 0);
    T b = matrix(0, 1);
    T c = matrix(0, 2);
    T d = matrix(1, 0);
    T e = matrix(1, 1);
    T f = matrix(1, 2);
    T g = matrix(2, 0);
    T h = matrix(2, 1);
    T i = matrix(2, 2);

    Matrix<T, 3, 3> adj({{e * i - f * h, -(d * i - f * g), d * h - e * g, -(b * i - c * h), a * i - c * g,
                          -(a * h - b * g), b * f - c * e, -(a * f - c * d), a * e - b * d}});

    return adj / det;
}

template <typename T>
constexpr Matrix<T, 4, 4> invert(const Matrix<T, 4, 4>& matrix)
{
    T det = determinant(matrix);
    if (abs(det) < std::numeric_limits<T>::epsilon())
    {
        log(LogLevel::WARNING, "Could not invert 4x4 matrix, determinant = 0");
        return matrix;
    }

    Matrix<T, 4, 4> adj;
    for (u64 row = 0; row < 4; ++row)
    {
        for (u64 col = 0; col < 4; ++col)
        {
            Matrix<T, 3, 3> subMatrix;
            u64 subRow = 0;
            for (u64 i = 0; i < 4; ++i)
            {
                if (i == row)
                {
                    continue;
                }
                u64 subCol = 0;
                for (u64 j = 0; j < 4; ++j)
                {
                    if (j == col)
                    {
                        continue;
                    }
                    subMatrix(subRow, subCol++) = matrix(i, j);
                }
                ++subRow;
            }
            adj(col, row) = ((row + col) % 2 == 0 ? 1 : -1) * determinant(subMatrix);
        }
    }

    return adj / det;
}

}