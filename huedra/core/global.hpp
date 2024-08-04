#pragma once

#include "core/timer.hpp"
#include "graphics/graphics_manager.hpp"
#include "window/window_manager.hpp"

namespace huedra {

struct Global
{
    static Timer timer;
    static WindowManager windowManager;
    static GraphicsManager graphicsManager;
};

} // namespace huedra
