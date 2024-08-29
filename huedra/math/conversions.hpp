#pragma once

#include <cmath>
#include <numbers>

namespace huedra {

constexpr double radians(double degrees) { return degrees * (std::numbers::pi / 180.0); }

constexpr float radians(float degrees) { return degrees * static_cast<float>(std::numbers::pi / 180.0); }

constexpr double degrees(double radians) { return radians * (180.0 / std::numbers::pi); }

constexpr float degrees(float radians) { return radians * static_cast<float>(180.0 / std::numbers::pi); }

} // namespace huedra