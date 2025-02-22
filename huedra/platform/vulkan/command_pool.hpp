#pragma once

#include "platform/vulkan/device.hpp"

namespace huedra {

class CommandPool
{
public:
    CommandPool() = default;
    ~CommandPool() = default;

    CommandPool(const CommandPool& rhs) = default;
    CommandPool& operator=(const CommandPool& rhs) = default;
    CommandPool(CommandPool&& rhs) = default;
    CommandPool& operator=(CommandPool&& rhs) = default;

    void init(Device& device, VkPipelineBindPoint pipeline);
    void cleanup();

    VkCommandBuffer beginSingleTimeCommand();
    void endSingleTimeCommand(VkCommandBuffer buffer);

    VkCommandPool get() { return m_commandPool; }

private:
    Device* m_device{nullptr};

    VkCommandPool m_commandPool{nullptr};
    VkPipelineBindPoint m_pipeline{VK_PIPELINE_BIND_POINT_GRAPHICS};
};

} // namespace huedra