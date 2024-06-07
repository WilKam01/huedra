#pragma once

#include "core/types.hpp"
#include <vulkan/vulkan.h>

namespace huedra::vulkan_config {

#ifdef DEBUG
const static bool enableValidationLayers = true;
#else
const static bool enableValidationLayers = false;
#endif

const static std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};

const static std::vector<const char*> windowExtensions = {
#ifdef DEBUG
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#endif

};

const static std::vector<const char*> deviceExtensions = {
    // TODO: Add swapchain support
    // VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

} // namespace huedra::vulkan_config