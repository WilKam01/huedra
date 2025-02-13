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
    {}

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
    constexpr std::strong_ordering operator<=>(const Vec3<T>& rhs) const
    {
        if (auto cmp = x <=> rhs.x; cmp != 0)
        {
            return cmp;
        }
        if (auto cmp = y <=> rhs.y; cmp != 0)
        {
            return cmp;
        }
        return z <=> rhs.z;
    }
    constexpr bool operator==(const Vec3<T>& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z; };
};

#define VEC_OP(OP)                                                         \
    template <typename T>                                                  \
    constexpr Vec3<T> operator OP(T scalar, const Vec3<T>& vec)            \
    {                                                                      \
        return Vec3<T>(scalar OP vec.x, scalar OP vec.y, scalar OP vec.z); \
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

using vec3 = Vec3<float>;
using ivec3 = Vec3<i32>;
using uvec3 = Vec3<u32>;
using bvec3 = Vec3<bool>;
using dvec3 = Vec3<double>;

} // namespace huedra