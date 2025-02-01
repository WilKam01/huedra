#pragma once

#include "platform/vulkan/device.hpp"

namespace huedra {

class CommandPool
{
public:
    CommandPool() = default;
    ~CommandPool() = default;

    void init(Device& device, VkPipelineBindPoint pipeline);
    void cleanup();

    VkCommandBuffer beginSingleTimeCommand();
    void endSingleTimeCommand(VkCommandBuffer buffer);

    void transistionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout,
                                VkImageLayout newLayout, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
                                VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask);

    VkCommandPool get() { return m_commandPool; }

private:
    Device* p_device;

    VkCommandPool m_commandPool;
    VkPipelineBindPoint m_pipeline;
};

} // namespace huedra