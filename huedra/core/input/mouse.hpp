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

enum class CursorType
{
    DEFAULT,
    CARET,
    WAIT,
    WAIT_IN_BACKGROUND,
    CROSSHAIR,
    HAND_POINT,
    HAND_OPEN,
    HAND_GRAB,
    HELP,
    NO_ENTRY,
    MOVE,
    SIZE_NS,   // North -> South
    SIZE_WE,   // West -> East
    SIZE_NWSE, // NorthWest -> SouthEast
    SIZE_NESW, // NorthEast -> SouthWest
};

} // namespace huedra
