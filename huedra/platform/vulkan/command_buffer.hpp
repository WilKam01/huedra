#pragma once

#include "platform/vulkan/command_pool.hpp"

namespace huedra {

class CommandBuffer
{
public:
    CommandBuffer() = default;
    ~CommandBuffer() = default;

    void init(Device& device, CommandPool& commandPool, u32 size);
    void cleanup();

    void begin(size_t index);
    void end(size_t index);

    size_t size() { return m_commandBuffers.size(); }
    VkCommandBuffer& get(size_t index) { return m_commandBuffers[index]; }

private:
    Device* p_device;
    CommandPool* p_commandPool;
    std::vector<VkCommandBuffer> m_commandBuffers;
};

} // namespace huedra