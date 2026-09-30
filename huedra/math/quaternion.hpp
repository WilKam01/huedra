#pragma once

#include "math/vec3.hpp"
#include "math/vec_transform.hpp"

namespace huedra {

class Quaternion
{
public:
    Quaternion() = default;
    constexpr Quaternion(f32 scalar, const vec3& axis) : scalar(scalar), axis(axis) {}

    constexpr Quaternion operator+(const Quaternion& rhs) const { return {scalar + rhs.scalar, axis + rhs.axis}; }
    constexpr Quaternion operator*(const Quaternion& rhs) const
    {
        return {(scalar * rhs.scalar) - math::dot(axis, rhs.axis),
                (scalar * rhs.axis) + (rhs.scalar * axis) + math::cross(axis, rhs.axis)};
    }

    constexpr Quaternion operator*(f32 scalar) const { return {this->scalar * scalar, axis * scalar}; }
    constexpr Quaternion operator/(f32 scalar) const { return {this->scalar / scalar, axis / scalar}; }
    constexpr Quaternion operator-() const { return {-scalar, -axis}; }

    operator std::string() const { return std::format("scalar: {}, axis: {}", scalar, axis); }
    std::string str() const { return std::format("scalar: {}, axis: {}", scalar, axis); }

    f32 scalar{0.0f};
    vec3 axis;
};

constexpr Quaternion operator*(f32 scalar, const Quaternion& quat)
{
    return {quat.scalar * scalar, quat.axis * scalar};
}

} // namespace huedra

template <>
struct std::formatter<huedra::Quaternion>
{
    std::string_view fmtSpec;
    constexpr auto parse(std::format_parse_context& ctx)
    {
        auto it = ctx.begin();
        auto end = ctx.end();

        while (it != end && *it != '}')
        {
            ++it;
        }

        if (ctx.begin() != it)
        {
            fmtSpec = std::string_view(ctx.begin(), it);
        }

        return it;
    }

    auto format(const huedra::Quaternion& quat, std::format_context& ctx) const
    {
        std::string elemFmt = "{}";
        if (!fmtSpec.empty())
        {
            elemFmt = "{" + std::string(1, ':') + std::string(fmtSpec) + "}";
        }
        auto out = ctx.out();
        out = std::format_to(out, "scalar: ");
        out = std::vformat_to(out, elemFmt, std::make_format_args(quat.scalar));
        out = std::format_to(out, ", axis: ");
        return std::vformat_to(out, elemFmt, std::make_format_args(quat.axis));
    }
};