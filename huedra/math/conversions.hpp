#pragma once

#include <cmath>
#include <concepts>
#include <numbers>

namespace huedra {

template <typename T>
    requires std::is_arithmetic_v<T>
constexpr float radians(T degrees)
{
    return static_cast<float>(degrees) * (std::numbers::pi / 180.0);
}

template <typename T>
    requires std::is_arithmetic_v<T>
constexpr float degrees(T radians)
{
    return static_cast<float>(radians) * (180.0 / std::numbers::pi);
}

} // namespace huedra