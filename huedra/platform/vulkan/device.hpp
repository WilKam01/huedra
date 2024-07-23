#pragma once

#include "platform/vulkan/instance.hpp"

namespace huedra {

struct QueueFamilyIndices
{
    std::optional<u32> graphicsFamily;
    std::optional<u32> presentFamily;
    std::optional<u32> computeFamily;

    bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value() && computeFamily.has_value(); }
};

struct VulkanSurfaceSupport
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class Device
{
public:
    Device() = default;
    ~Device() = default;

    void init(Instance& instance, VkSurfaceKHR surface);
    void cleanup();

    void waitIdle();
    u32 findMemoryType(u32 typeBits, VkMemoryPropertyFlags properties);
    VulkanSurfaceSupport querySurfaceSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
    VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    VkDevice getLogical() { return m_device; }
    VkPhysicalDevice getPhysical() { return m_physicalDevice; }
    VkQueue getGraphicsQueue() { return m_graphicsQueue; }
    VkQueue getPresentQueue() { return m_presentQueue; }
    VkQueue getComputeQueue() { return m_computeQueue; }
    QueueFamilyIndices getQueueFamilyIndices() { return m_indices; }
    VkSampleCountFlagBits getMsaaSamples() { return m_msaaSamples; }

private:
    void pickPhysicalDevice(Instance& instance, VkSurfaceKHR surface);
    bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    void createLogicalDevice(VkSurfaceKHR surface);
    VkSampleCountFlagBits getMaxUsableSampleCount();

    VkDevice m_device;
    VkPhysicalDevice m_physicalDevice;

    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;
    VkQueue m_computeQueue;
    QueueFamilyIndices m_indices;

    VkSampleCountFlagBits m_msaaSamples{VK_SAMPLE_COUNT_1_BIT};
};

} // namespace huedra
