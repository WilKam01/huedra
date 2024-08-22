#pragma once

#include "math/vec.hpp"

namespace huedra {

template <typename T>
struct Vec2
{
public:
    Vec2() = default;
    Vec2(T val)
    {
        x = val;
        y = val;
    }
    Vec2(T x, T y)
    {
        this->x = x;
        this->y = y;
    }

    template <u64 L>
    Vec2(Vec<T, L> vec) : data(vec)
    {
    }

    union
    {
        Vec<T, 2> data;
        struct
        {
            T x, y;
        };
        struct
        {
            T r, g;
        };
        struct
        {
            T s, t;
        };
    };

    inline T operator[](u64 index) const { return data[index]; }
    inline T& operator[](u64 index) { return data[index]; }

#define VEC_OP(OP)                                                                                \
    constexpr Vec2<T> operator OP(const Vec2<T>& rhs) { return Vec2<T>(x OP rhs.x, y OP rhs.y); } \
                                                                                                  \
    constexpr Vec2<T>& operator OP##=(const Vec2<T>& rhs)                                         \
    {                                                                                             \
        x OP## = rhs.x;                                                                           \
        y OP## = rhs.y;                                                                           \
        return *this;                                                                             \
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

    constexpr Vec2<T> operator-() const { return Vec2<T>(-x, -y); }
    constexpr bool operator==(const Vec2<T>& rhs) const
    {
        if (x != rhs.x)
        {
            return false;
        }
        if (y != rhs.y)
        {
            return false;
        }
        return true;
    }
};

using vec2 = Vec2<float>;
using ivec2 = Vec2<i32>;
using uvec2 = Vec2<u32>;
using bvec2 = Vec2<bool>;
using dvec2 = Vec2<double>;

} // namespace huedra