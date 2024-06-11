#pragma once

#ifdef WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include "core/types.hpp"
#include <vulkan/vulkan.h>

namespace huedra::vulkan_config {

#ifdef DEBUG
const static bool enableValidationLayers = true;
#else
const static bool enableValidationLayers = false;
#endif

const static std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};

const static std::vector<const char*> instanceExtensions = {

    VK_KHR_SURFACE_EXTENSION_NAME,

#ifdef WIN32
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif

#ifdef DEBUG
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#endif

};

const static std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

} // namespace huedra::vulkan_config