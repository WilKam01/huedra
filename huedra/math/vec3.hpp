#pragma once

#include "math/vec.hpp"

namespace huedra {

template <typename T>
struct Vec3
{
public:
    Vec3() = default;
    explicit Vec3(T val) : x(val), y(val), z(val) {}
    Vec3(T x, T y, T z) : x(x), y(y), z(z) {}

    template <typename T2, u64 L>
        requires(std::is_convertible_v<T, T2>)
    explicit Vec3(const Vec<T2, L>& vec) : data(vec)
    {}

    template <typename T2>
        requires(std::is_convertible_v<T, T2>)
    explicit Vec3(const Vec3<T2>& vec) : x(static_cast<T>(vec.x)), y(static_cast<T>(vec.y)), z(static_cast<T>(vec.z))
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

    T operator[](u64 index) const { return data[index]; }
    T& operator[](u64 index) { return data[index]; }

    // No better option than using macros for easy operator overloading
    // NOLINTBEGIN(cppcoreguidelines-macro-usage)
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
    // NOLINTEND(cppcoreguidelines-macro-usage)

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

// No better option than using macros for easy operator overloading
// NOLINTBEGIN(cppcoreguidelines-macro-usage)
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
// NOLINTEND(cppcoreguidelines-macro-usage)

using vec3 = Vec3<float>;
using ivec3 = Vec3<i32>;
using uvec3 = Vec3<u32>;
using bvec3 = Vec3<bool>;
using dvec3 = Vec3<double>;

} // namespace huedra