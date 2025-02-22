#pragma once

#include "core/input/input.hpp"
#include "core/timer.hpp"
#include "graphics/graphics_manager.hpp"
#include "resources/resource_manager.hpp"
#include "scene/scene_manager.hpp"
#include "window/window_manager.hpp"

namespace huedra::global {

inline Timer timer;
inline Input input;
inline WindowManager windowManager;
inline GraphicsManager graphicsManager;
inline ResourceManager resourceManager;
inline SceneManager sceneManager;

} // namespace huedra::global