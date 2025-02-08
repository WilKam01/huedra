#pragma once

#include "core/input/input.hpp"
#include "core/timer.hpp"
#include "graphics/graphics_manager.hpp"
#include "resources/resource_manager.hpp"
#include "window/window_manager.hpp"

namespace huedra {

struct Global
{
    static inline Timer timer;
    static inline Input input;
    static inline WindowManager windowManager;
    static inline GraphicsManager graphicsManager;
    static inline ResourceManager resourceManager;
};

} // namespace huedra
