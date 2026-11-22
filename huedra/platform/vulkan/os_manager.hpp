#pragma once

#include "huedra/platform/vulkan/config.hpp"
#include "huedra/platform/vulkan/instance.hpp"
#include "huedra/window/window.hpp"

namespace huedra {

VkSurfaceKHR createSurface(Instance& instance, Window* window);

}