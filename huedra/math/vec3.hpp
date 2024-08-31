#pragma once

#include "math/vec.hpp"

namespace huedra {

template <typename T>
struct Vec3
{
public:
    Vec3() = default;
    Vec3(T val)
    {
        x = val;
        y = val;
        z = val;
    }
    Vec3(T x, T y, T z)
    {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    template <u64 L>
    Vec3(Vec<T, L> vec) : data(vec)
    {
    }

    union
    {
        Vec<T, 3> data{};
        struct
        {
            T x, y, z;
        };
        struct
        {
            T r, g, b;
        };
        struct
        {
            T s, t, p;
        };
    };

    inline T operator[](u64 index) const { return data[index]; }
    inline T& operator[](u64 index) { return data[index]; }

#define VEC_OP(OP)                                                                                                  \
    constexpr Vec3<T> operator OP(const Vec3<T>& rhs) const { return Vec3<T>(x OP rhs.x, y OP rhs.y, z OP rhs.z); } \
                                                                                                                    \
    constexpr Vec3<T> operator OP(T scalar) const { return Vec3<T>(x OP scalar, y OP scalar, z OP scalar); }        \
                                                                                                                    \
    constexpr Vec3<T>& operator OP##=(const Vec3<T>& rhs)                                                           \
    {                                                                                                               \
        x OP## = rhs.x;                                                                                             \
        y OP## = rhs.y;                                                                                             \
        z OP## = rhs.z;                                                                                             \
        return *this;                                                                                               \
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

    constexpr Vec3<T> operator-() const { return Vec3<T>(-x, -y, -z); }
    constexpr bool operator==(const Vec3<T>& rhs) const
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
        return true;
    }
};

using vec3 = Vec3<float>;
using ivec3 = Vec3<i32>;
using uvec3 = Vec3<u32>;
using bvec3 = Vec3<bool>;
using dvec3 = Vec3<double>;

} // namespace huedra