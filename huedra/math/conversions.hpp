#pragma once

#include <cmath>
#include <concepts>
#include <numbers>

namespace huedra::math {

template <typename T>
    requires std::is_arithmetic_v<T>
constexpr f32 radians(T degrees)
{
    return static_cast<f32>(degrees) * (std::numbers::pi / 180.0);
}

template <typename T>
    requires std::is_arithmetic_v<T>
constexpr f32 degrees(T radians)
{
    return static_cast<f32>(radians) * (180.0 / std::numbers::pi);
}

} // namespace huedra