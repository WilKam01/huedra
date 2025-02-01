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

const static std::map<VkImageLayout, VkAccessFlagBits> layoutToAccess = {
    {VK_IMAGE_LAYOUT_UNDEFINED, VK_ACCESS_NONE},
    {VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT},
    {VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_SHADER_WRITE_BIT},
    {VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT},
    {VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT},
    {VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT},
    {VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_TRANSFER_READ_BIT},
    {VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT},
    {VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_ACCESS_NONE}};

} // namespace huedra::vulkan_config