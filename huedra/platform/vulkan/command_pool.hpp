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

    VkCommandPool get() { return m_commandPool; }

private:
    Device* p_device;

    VkCommandPool m_commandPool;
    VkPipelineBindPoint m_pipeline;
};

}