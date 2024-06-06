#include "instance.hpp"

#include <string.h>

namespace huedra {

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                    VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                    void* pUserData)
{
    // TODO: Log message
    return VK_FALSE;
}

void Instance::init()
{
    if (c_enableValidationLayers && !checkValidationLayerSupport())
    {
        // TODO: Log runtime error
    }

    VkApplicationInfo appInfo{VK_STRUCTURE_TYPE_APPLICATION_INFO};
    appInfo.pNext = nullptr;
    appInfo.pEngineName = "huedra";
    appInfo.pEngineName = "appName"; // TODO: Add actual name
    appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<u32>(c_windowExtensions.size());
    createInfo.ppEnabledExtensionNames = c_windowExtensions.data();
    createInfo.enabledLayerCount = 0;
    createInfo.pNext = nullptr;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (c_enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<u32>(c_validationLayers.size());
        createInfo.ppEnabledLayerNames = c_validationLayers.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }

    if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS)
    {
        // TODO: Log runtime error
    }

#ifdef DEBUG
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    // TODO: Log available extensions
#endif

    if (c_enableValidationLayers)
    {
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
        populateDebugMessengerCreateInfo(debugCreateInfo);

        if (createDebugUtilsMessengerEXT(&debugCreateInfo, nullptr, &m_debugMessenger) != VK_SUCCESS)
        {
            // TODO: Log runtime error
        }
    }
}

void Instance::cleanup()
{
    if (c_enableValidationLayers)
    {
        destroyDebugUtilsMessengerEXT(m_debugMessenger, nullptr);
    }

    vkDestroyInstance(m_instance, nullptr);
}

void Instance::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

VkResult Instance::createDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator,
                                                VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT"));
    if (func != nullptr)
    {
        return func(m_instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void Instance::destroyDebugUtilsMessengerEXT(VkDebugUtilsMessengerEXT debugMessenger,
                                             const VkAllocationCallbacks* pAllocator)
{
    auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT"));
    if (func != nullptr)
    {
        func(m_instance, debugMessenger, pAllocator);
    }
}

bool Instance::checkValidationLayerSupport()
{
    u32 count = 0;
    vkEnumerateInstanceLayerProperties(&count, nullptr);
    std::vector<VkLayerProperties> availableLayers(count);
    vkEnumerateInstanceLayerProperties(&count, availableLayers.data());

    for (const char* name : c_validationLayers)
    {
        bool found = false;

        for (const auto& props : availableLayers)
        {
            if (strcmp(name, props.layerName) == 0)
            {
                found = true;
                break;
            }
        }

        if (!found)
            return false;
    }

    return true;
}

} // namespace huedra