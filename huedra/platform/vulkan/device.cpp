#include "device.hpp"

#include "core/log.hpp"

#include <set>

namespace huedra {

void Device::init(Instance& instance)
{
    pickPhysicalDevice(instance);
    createLogicalDevice();
}

void Device::cleanup() { vkDestroyDevice(m_device, nullptr); }

void Device::waitIdle() { vkDeviceWaitIdle(m_device); }

u32 Device::findMemoryType(u32 typeBits, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

    for (u32 i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if (typeBits & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }

    log(LogLevel::ERROR, "Failed to find suitable memory type!");
    return 0;
}

void Device::pickPhysicalDevice(Instance& instance)
{
    m_physicalDevice = VK_NULL_HANDLE;

    u32 count = 0;
    vkEnumeratePhysicalDevices(instance.get(), &count, nullptr);
    std::vector<VkPhysicalDevice> devices(count);
    vkEnumeratePhysicalDevices(instance.get(), &count, devices.data());

    for (const auto& device : devices)
    {
        if (isDeviceSuitable(device))
        {
            m_physicalDevice = device;
            m_msaaSamples = getMaxUsableSampleCount();
            break;
        }
    }

    if (m_physicalDevice == VK_NULL_HANDLE)
    {
        log(LogLevel::ERROR, "Failed to find suitable GPU!");
    }
}

bool Device::isDeviceSuitable(VkPhysicalDevice device)
{
    QueueFamilyIndices indices = findQueueFamilies(device);

    bool extensionsSupported = checkDeviceExtensionSupport(device);
    // TODO: Check swapchain support
    bool swapChainAdequate = true; /*false;
    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport = SwapChain::querySupport(device, *p_surface);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }*/

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy &&
           supportedFeatures.fillModeNonSolid;
}

QueueFamilyIndices Device::findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;

    u32 count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, queueFamilies.data());

    int i = 0;
    indices.presentFamily = 0; // TODO: Check present support on window
    for (const auto& queueFamily : queueFamilies)
    {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.graphicsFamily = i;
        if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
            indices.computeFamily = i;

        // TODO: Check present support on window
        /*VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, *p_surface, &presentSupport);
        if (presentSupport)
            indices.presentFamily = i;*/

        if (indices.isComplete())
            break;
        i++;
    }

    return indices;
}

bool Device::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
    u32 count = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, availableExtensions.data());

    std::set<std::string> requiredExtensions(vulkan_config::deviceExtensions.begin(),
                                             vulkan_config::deviceExtensions.end());
    for (const auto& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

void Device::createLogicalDevice()
{
    m_indices = findQueueFamilies(m_physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<u32> uniqueQueueFamilies = {m_indices.graphicsFamily.value(), m_indices.presentFamily.value()};

    float queuePriority = 1.0f;
    for (u32 queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures2 deviceFeatures{};
    deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    deviceFeatures.pNext = nullptr;
    deviceFeatures.features.samplerAnisotropy = VK_TRUE;
    deviceFeatures.features.sampleRateShading = VK_TRUE;
    deviceFeatures.features.fillModeNonSolid = VK_TRUE;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = &deviceFeatures;
    createInfo.queueCreateInfoCount = static_cast<u32>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = nullptr;
    createInfo.enabledExtensionCount = static_cast<u32>(vulkan_config::deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = vulkan_config::deviceExtensions.data();

    createInfo.enabledLayerCount = 0;
    if (vulkan_config::enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(vulkan_config::validationLayers.size());
        createInfo.ppEnabledLayerNames = vulkan_config::validationLayers.data();
    }

    if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS)
    {
        log(LogLevel::ERROR, "Failed to create logical device!");
    }

    vkGetDeviceQueue(m_device, m_indices.graphicsFamily.value(), 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, m_indices.presentFamily.value(), 0, &m_presentQueue);
    vkGetDeviceQueue(m_device, m_indices.computeFamily.value(), 0, &m_computeQueue);
}

VkSampleCountFlagBits Device::getMaxUsableSampleCount()
{
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(m_physicalDevice, &physicalDeviceProperties);

    VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts &
                                physicalDeviceProperties.limits.framebufferDepthSampleCounts;

    if (counts & VK_SAMPLE_COUNT_64_BIT)
    {
        return VK_SAMPLE_COUNT_64_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_32_BIT)
    {
        return VK_SAMPLE_COUNT_32_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_16_BIT)
    {
        return VK_SAMPLE_COUNT_16_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_8_BIT)
    {
        return VK_SAMPLE_COUNT_8_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_4_BIT)
    {
        return VK_SAMPLE_COUNT_4_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_2_BIT)
    {
        return VK_SAMPLE_COUNT_2_BIT;
    }

    return VK_SAMPLE_COUNT_1_BIT;
}

} // namespace huedra
