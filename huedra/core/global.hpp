#pragma once

#include "huedra/core/input/input.hpp"
#include "huedra/core/timer.hpp"
#include "huedra/graphics/graphics_manager.hpp"
#include "huedra/resources/resource_manager.hpp"
#include "huedra/scene/scene_manager.hpp"
#include "huedra/window/window_manager.hpp"

namespace huedra::global {

inline Timer timer;
inline Input input;
inline WindowManager windowManager;
inline GraphicsManager graphicsManager;
inline ResourceManager resourceManager;
inline SceneManager sceneManager;

} // namespace huedra::global