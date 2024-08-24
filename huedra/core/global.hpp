#pragma once

#include "core/timer.hpp"
#include "graphics/graphics_manager.hpp"
#include "window/window_manager.hpp"

namespace huedra {

struct Global
{
    static inline Timer timer;
    static inline WindowManager windowManager;
    static inline GraphicsManager graphicsManager;
};

} // namespace huedra
