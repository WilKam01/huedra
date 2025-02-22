#pragma once

#include "platform/vulkan/config.hpp"

namespace huedra {

class Instance
{
public:
    Instance() = default;
    ~Instance() = default;

    Instance(const Instance& rhs) = default;
    Instance& operator=(const Instance& rhs) = default;
    Instance(Instance&& rhs) = default;
    Instance& operator=(Instance&& rhs) = default;

    void init();
    void cleanup();

    VkInstance get() { return m_instance; }

private:
    static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    VkResult createDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator,
                                          VkDebugUtilsMessengerEXT* pDebugMessenger);
    void destroyDebugUtilsMessengerEXT(VkDebugUtilsMessengerEXT debugMessenger,
                                       const VkAllocationCallbacks* pAllocator);

    static bool checkValidationLayerSupport();

    VkInstance m_instance{nullptr};
    VkDebugUtilsMessengerEXT m_debugMessenger{nullptr};
};

} // namespace huedra
