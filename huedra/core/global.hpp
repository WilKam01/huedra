#pragma once

#include "graphics/graphics_manager.hpp"
#include "window/window_manager.hpp"

namespace huedra {

struct Global
{
    static WindowManager windowManager;
    static GraphicsManager graphicsManager;
};

} // namespace huedra
