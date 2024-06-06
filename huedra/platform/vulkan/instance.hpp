#pragma once

#include "core/types.hpp"

#include <vulkan/vulkan.h>

namespace huedra {

class Instance
{
public:
    Instance() = default;
    ~Instance() = default;

    void init();
    void cleanup();

    inline VkInstance get() { return m_instance; }
    inline bool validationLayersEnabled() { return c_enableValidationLayers; }

private:
#ifdef DEBUG
    const bool c_enableValidationLayers = true;
#else
    const bool c_enableValidationLayers = false;
#endif

    const std::vector<const char*> c_validationLayers = {"VK_LAYER_KHRONOS_validation"};

    const std::vector<const char*> c_windowExtensions = {
#ifdef DEBUG
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#endif
    };

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    VkResult createDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator,
                                          VkDebugUtilsMessengerEXT* pDebugMessenger);
    void destroyDebugUtilsMessengerEXT(VkDebugUtilsMessengerEXT debugMessenger,
                                       const VkAllocationCallbacks* pAllocator);

    bool checkValidationLayerSupport();

    VkInstance m_instance;
    VkDebugUtilsMessengerEXT m_debugMessenger;
};

} // namespace huedra
