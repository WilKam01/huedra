#pragma once

#include "math/vec.hpp"

namespace huedra {

template <typename T>
struct Vec4
{
public:
    Vec4() = default;
    explicit Vec4(T val) : x(val), y(val), z(val), w(val) {}
    Vec4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}

    template <typename T2, u64 L>
        requires(std::is_convertible_v<T, T2>)
    explicit Vec4(const Vec<T2, L>& vec) : data(vec)
    {}

    template <typename T2>
        requires(std::is_convertible_v<T, T2>)
    explicit Vec4(const Vec4<T2>& vec)
        : x(static_cast<T>(vec.x)), y(static_cast<T>(vec.y)), z(static_cast<T>(vec.z)), w(static_cast<T>(vec.w))
    {}

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

    T operator[](u64 index) const { return data[index]; }
    T& operator[](u64 index) { return data[index]; }

    // No better option than using macros for easy operator overloading
    // NOLINTBEGIN(cppcoreguidelines-macro-usage)
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
    // NOLINTEND(cppcoreguidelines-macro-usage)

    constexpr Vec4<T> operator-() const { return Vec4<T>(-x, -y, -z); }
    constexpr std::strong_ordering operator<=>(const Vec4<T>& rhs) const
    {
        if (auto cmp = x <=> rhs.x; cmp != 0)
        {
            return cmp;
        }
        if (auto cmp = y <=> rhs.y; cmp != 0)
        {
            return cmp;
        }
        if (auto cmp = z <=> rhs.z; cmp != 0)
        {
            return cmp;
        }
        return w <=> rhs.w;
    }
    constexpr bool operator==(const Vec4<T>& rhs) const
    {
        return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
    };
};

// No better option than using macros for easy operator overloading
// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define VEC_OP(OP)                                                                          \
    template <typename T>                                                                   \
    constexpr Vec4<T> operator OP(T scalar, const Vec4<T>& vec)                             \
    {                                                                                       \
        return Vec4<T>(scalar OP vec.x, scalar OP vec.y, scalar OP vec.z, scalar OP vec.w); \
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
// NOLINTEND(cppcoreguidelines-macro-usage)

using vec4 = Vec4<float>;
using ivec4 = Vec4<i32>;
using uvec4 = Vec4<u32>;
using bvec4 = Vec4<bool>;
using dvec4 = Vec4<double>;

using i8vec4 = Vec4<i8>;
using u8vec4 = Vec4<u8>;
using i16vec4 = Vec4<i16>;
using u16vec4 = Vec4<u16>;
using i32vec4 = Vec4<i32>;
using u32vec4 = Vec4<u32>;
using i64vec4 = Vec4<i64>;
using u64vec4 = Vec4<u64>;

} // namespace huedra