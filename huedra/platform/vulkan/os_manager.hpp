#pragma once

#include "platform/vulkan/config.hpp"
#include "platform/vulkan/instance.hpp"
#include "window/window.hpp"

namespace huedra {

VkSurfaceKHR createSurface(Instance& instance, Window* window);

}