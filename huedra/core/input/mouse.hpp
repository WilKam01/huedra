#pragma once

namespace huedra {

enum class MouseButton
{
    NONE = -1,
    LEFT,
    RIGHT,
    MIDDLE,
    EXTRA1,
    EXTRA2
};

enum class MouseMode
{
    DISABLED,
    NORMAL,
    LOCKED,
    CONFINED
};

} // namespace huedra
