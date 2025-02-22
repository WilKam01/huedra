#pragma once

#include "platform/vulkan/instance.hpp"

namespace huedra {

struct QueueFamilyIndices
{
    std::optional<u32> graphicsFamily;
    std::optional<u32> presentFamily;
    std::optional<u32> computeFamily;

    bool isComplete() const
    {
        return graphicsFamily.has_value() && presentFamily.has_value() && computeFamily.has_value();
    }
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

    Device(const Device& rhs) = default;
    Device& operator=(const Device& rhs) = default;
    Device(Device&& rhs) = default;
    Device& operator=(Device&& rhs) = default;

    void init(Instance& instance, VkSurfaceKHR surface);
    void cleanup();

    void waitIdle();
    u32 findMemoryType(u32 typeBits, VkMemoryPropertyFlags properties);
    static VulkanSurfaceSupport querySurfaceSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
    static VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkFormat findDepthFormat();

    static void transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format,
                                      VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccessMask,
                                      VkAccessFlags dstAccessMask, VkPipelineStageFlags srcStageMask,
                                      VkPipelineStageFlags dstStageMask);

    VkDevice getLogical() const { return m_device; }
    VkPhysicalDevice getPhysical() const { return m_physicalDevice; }
    VkQueue getGraphicsQueue() const { return m_graphicsQueue; }
    VkQueue getPresentQueue() const { return m_presentQueue; }
    VkQueue getComputeQueue() const { return m_computeQueue; }
    u32 getGraphicsQueueFamilyIndex() const { return m_graphicsQueueuFamilyIndex; }
    u32 getPresentQueueFamilyIndex() const { return m_presentQueueuFamilyIndex; }
    u32 getComputeQueueFamilyIndex() const { return m_computeQueueuFamilyIndex; }
    VkSampleCountFlagBits getMsaaSamples() const { return m_msaaSamples; }

private:
    void pickPhysicalDevice(Instance& instance, VkSurfaceKHR surface);
    static bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);
    static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
    static bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    void createLogicalDevice(VkSurfaceKHR surface);
    VkSampleCountFlagBits getMaxUsableSampleCount();

    VkDevice m_device{nullptr};
    VkPhysicalDevice m_physicalDevice{nullptr};

    VkQueue m_graphicsQueue{nullptr};
    VkQueue m_presentQueue{nullptr};
    VkQueue m_computeQueue{nullptr};
    u32 m_graphicsQueueuFamilyIndex{0};
    u32 m_presentQueueuFamilyIndex{0};
    u32 m_computeQueueuFamilyIndex{0};

    VkSampleCountFlagBits m_msaaSamples{VK_SAMPLE_COUNT_1_BIT};
};

} // namespace huedra
