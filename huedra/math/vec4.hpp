#pragma once

#include "math/vec.hpp"

namespace huedra {

template <typename T>
struct Vec4
{
public:
    Vec4() = default;
    Vec4(T val)
    {
        x = val;
        y = val;
        z = val;
        w = val;
    }
    Vec4(T x, T y, T z, T w)
    {
        this->x = x;
        this->y = y;
        this->z = z;
        this->w = w;
    }

    template <u64 L>
    Vec4(Vec<T, L> vec) : data(vec)
    {
    }

    union
    {
        Vec<T, 4> data{};
        struct
        {
            T x, y, z, w;
        };
        struct
        {
            T r, g, b, a;
        };
        struct
        {
            T s, t, p, q;
        };
    };

    inline T operator[](u64 index) const { return data[index]; }
    inline T& operator[](u64 index) { return data[index]; }

#define VEC_OP(OP)                                                          \
    constexpr Vec4<T> operator OP(const Vec4<T>& rhs) const                 \
    {                                                                       \
        return Vec4<T>(x OP rhs.x, y OP rhs.y, z OP rhs.z, w OP rhs.w);     \
    }                                                                       \
                                                                            \
    constexpr Vec4<T> operator OP(T scalar) const                           \
    {                                                                       \
        return Vec4<T>(x OP scalar, y OP scalar, z OP scalar, w OP scalar); \
    }                                                                       \
                                                                            \
    constexpr Vec4<T>& operator OP##=(const Vec4<T>& rhs)                   \
    {                                                                       \
        x OP## = rhs.x;                                                     \
        y OP## = rhs.y;                                                     \
        z OP## = rhs.z;                                                     \
        w OP## = rhs.w;                                                     \
        return *this;                                                       \
    }

    VEC_OP(+);
    VEC_OP(-);
    VEC_OP(*);
    VEC_OP(/);
    VEC_OP(&);
    VEC_OP(|);
    VEC_OP(^);
    VEC_OP(>>);
    VEC_OP(<<);
#undef VEC_OP

    constexpr Vec4<T> operator-() const { return Vec4<T>(-x, -y, -z); }
    constexpr bool operator==(const Vec4<T>& rhs) const
    {
        if (x != rhs.x)
        {
            return false;
        }
        if (y != rhs.y)
        {
            return false;
        }
        if (z != rhs.z)
        {
            return false;
        }
        if (w != rhs.w)
        {
            return false;
        }
        return true;
    }
};

using vec4 = Vec4<float>;
using ivec4 = Vec4<i32>;
using uvec4 = Vec4<u32>;
using bvec4 = Vec4<bool>;
using dvec4 = Vec4<double>;

} // namespace huedra