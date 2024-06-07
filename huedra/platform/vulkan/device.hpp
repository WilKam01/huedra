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

class Device
{
public:
    Device() = default;
    ~Device() = default;

    void init(Instance& instance);
    void cleanup();

    void waitIdle();
    u32 findMemoryType(u32 typeBits, VkMemoryPropertyFlags properties);

    VkDevice getLogical() { return m_device; }
    VkPhysicalDevice getPhysical() { return m_physicalDevice; }
    VkQueue getGraphicsQueue() { return m_graphicsQueue; }
    VkQueue getPresentQueue() { return m_presentQueue; }
    VkQueue getComputeQueue() { return m_computeQueue; }
    QueueFamilyIndices getQueueFamilyIndices() { return m_indices; }
    VkSampleCountFlagBits getMsaaSamples() { return m_msaaSamples; }

private:
    void pickPhysicalDevice(Instance& instance);
    bool isDeviceSuitable(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    void createLogicalDevice();
    VkSampleCountFlagBits getMaxUsableSampleCount();

    VkDevice m_device;
    VkPhysicalDevice m_physicalDevice;

    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;
    VkQueue m_computeQueue;
    QueueFamilyIndices m_indices;

    VkSampleCountFlagBits m_msaaSamples = VK_SAMPLE_COUNT_1_BIT;
};

} // namespace huedra
