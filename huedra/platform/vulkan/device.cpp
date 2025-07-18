#include "device.hpp"

#include "core/log.hpp"

#include <set>

namespace huedra {

void Device::init(Instance& instance, VkSurfaceKHR surface)
{
    pickPhysicalDevice(instance, surface);
    createLogicalDevice(surface);
}

void Device::cleanup() { vkDestroyDevice(m_device, nullptr); }

void Device::waitIdle() { vkDeviceWaitIdle(m_device); }

u32 Device::findMemoryType(u32 typeBits, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

    for (u32 i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeBits & (1 << i)) != 0u && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    log(LogLevel::ERR, "Failed to find suitable memory type!");
    return 0;
}

VulkanSurfaceSupport Device::querySurfaceSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    VulkanSurfaceSupport details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount{0};
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount{0};
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR Device::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    VkSurfaceFormatKHR secondaryFormatChoice = availableFormats[0];
    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_R8G8B8A8_UNORM &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
        else if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
                 availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            secondaryFormatChoice = availableFormat;
        }
    }

    return secondaryFormatChoice;
}

VkFormat Device::findDepthFormat()
{
    std::vector<VkFormat> candidates{VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};
    VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
    VkFormatFeatureFlags features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

    for (VkFormat vkFormat : candidates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(m_physicalDevice, vkFormat, &props);

        if ((tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) ||
            (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features))
        {
            return vkFormat;
        }
    }

    log(LogLevel::ERR, "Failed to find supported depth format!");
    return VK_FORMAT_UNDEFINED;
}

void Device::pickPhysicalDevice(Instance& instance, VkSurfaceKHR surface)
{
    m_physicalDevice = VK_NULL_HANDLE;

    u32 count = 0;
    vkEnumeratePhysicalDevices(instance.get(), &count, nullptr);
    std::vector<VkPhysicalDevice> devices(count);
    vkEnumeratePhysicalDevices(instance.get(), &count, devices.data());

    for (const auto& device : devices)
    {
        if (isDeviceSuitable(device, surface))
        {
            m_physicalDevice = device;
            m_msaaSamples = getMaxUsableSampleCount();
            break;
        }
    }

    if (m_physicalDevice == VK_NULL_HANDLE)
    {
        log(LogLevel::ERR, "Failed to find suitable GPU!");
    }
}

bool Device::isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    QueueFamilyIndices indices = findQueueFamilies(device, surface);

    bool extensionsSupported = checkDeviceExtensionSupport(device);
    bool swapchainAdequate = false;
    if (extensionsSupported)
    {
        VulkanSurfaceSupport surfaceSupport = querySurfaceSupport(device, surface);
        swapchainAdequate = !surfaceSupport.formats.empty() && !surfaceSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return indices.isComplete() && extensionsSupported && swapchainAdequate &&
           supportedFeatures.samplerAnisotropy != 0u && supportedFeatures.fillModeNonSolid != 0u;
}

QueueFamilyIndices Device::findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    QueueFamilyIndices indices;

    u32 count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, queueFamilies.data());

    int i{0};
    for (const auto& queueFamily : queueFamilies)
    {
        if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0u)
        {
            indices.graphicsFamily = i;
        }
        if ((queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0u)
        {
            indices.computeFamily = i;
        }

        VkBool32 presentSupport = 0u;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if (presentSupport != 0u)
        {
            indices.presentFamily = i;
        }

        if (indices.isComplete())
        {
            break;
        }
        ++i;
    }

    return indices;
}

bool Device::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
    u32 count{0};
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, availableExtensions.data());

    std::set<std::string> requiredExtensions(vulkan_config::DEVICE_EXTENSIONS.begin(),
                                             vulkan_config::DEVICE_EXTENSIONS.end());
    for (const auto& extension : availableExtensions)
    {
        requiredExtensions.erase(static_cast<const char*>(extension.extensionName));
    }

    return requiredExtensions.empty();
}

void Device::createLogicalDevice(VkSurfaceKHR surface)
{
    QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice, surface);
    m_graphicsQueueuFamilyIndex = indices.graphicsFamily.value_or(~0u);
    m_presentQueueuFamilyIndex = indices.presentFamily.value_or(~0u);
    m_computeQueueuFamilyIndex = indices.computeFamily.value_or(~0u);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<u32> uniqueQueueFamilies = {m_graphicsQueueuFamilyIndex, m_presentQueueuFamilyIndex,
                                         m_computeQueueuFamilyIndex};

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
    createInfo.enabledExtensionCount = static_cast<u32>(vulkan_config::DEVICE_EXTENSIONS.size());
    createInfo.ppEnabledExtensionNames = vulkan_config::DEVICE_EXTENSIONS.data();

    createInfo.enabledLayerCount = 0;
    if (vulkan_config::ENABLE_VALIDATION_LAYERS)
    {
        createInfo.enabledLayerCount = static_cast<u32>(vulkan_config::VALIDATION_LAYERS.size());
        createInfo.ppEnabledLayerNames = vulkan_config::VALIDATION_LAYERS.data();
    }

    if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS)
    {
        log(LogLevel::ERR, "Failed to create logical device!");
    }

    vkGetDeviceQueue(m_device, m_graphicsQueueuFamilyIndex, 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, m_presentQueueuFamilyIndex, 0, &m_presentQueue);
    vkGetDeviceQueue(m_device, m_computeQueueuFamilyIndex, 0, &m_computeQueue);
}

VkSampleCountFlagBits Device::getMaxUsableSampleCount()
{
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(m_physicalDevice, &physicalDeviceProperties);

    VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts &
                                physicalDeviceProperties.limits.framebufferDepthSampleCounts;

    if ((counts & VK_SAMPLE_COUNT_64_BIT) != 0u)
    {
        return VK_SAMPLE_COUNT_64_BIT;
    }
    if ((counts & VK_SAMPLE_COUNT_32_BIT) != 0u)
    {
        return VK_SAMPLE_COUNT_32_BIT;
    }
    if ((counts & VK_SAMPLE_COUNT_16_BIT) != 0u)
    {
        return VK_SAMPLE_COUNT_16_BIT;
    }
    if ((counts & VK_SAMPLE_COUNT_8_BIT) != 0u)
    {
        return VK_SAMPLE_COUNT_8_BIT;
    }
    if ((counts & VK_SAMPLE_COUNT_4_BIT) != 0u)
    {
        return VK_SAMPLE_COUNT_4_BIT;
    }
    if ((counts & VK_SAMPLE_COUNT_2_BIT) != 0u)
    {
        return VK_SAMPLE_COUNT_2_BIT;
    }

    return VK_SAMPLE_COUNT_1_BIT;
}

} // namespace huedra
