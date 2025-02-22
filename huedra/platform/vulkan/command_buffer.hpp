#pragma once

#include "platform/vulkan/command_pool.hpp"

namespace huedra {

class CommandBuffer
{
public:
    CommandBuffer() = default;
    ~CommandBuffer() = default;

    CommandBuffer(const CommandBuffer& rhs) = default;
    CommandBuffer& operator=(const CommandBuffer& rhs) = default;
    CommandBuffer(CommandBuffer&& rhs) = default;
    CommandBuffer& operator=(CommandBuffer&& rhs) = default;

    void init(Device& device, CommandPool& commandPool, u32 size);
    void cleanup();

    void begin(u64 index);
    void end(u64 index);

    u64 size() { return m_commandBuffers.size(); }
    VkCommandBuffer& get(u64 index) { return m_commandBuffers[index]; }

private:
    Device* m_device{nullptr};
    CommandPool* m_commandPool{nullptr};
    std::vector<VkCommandBuffer> m_commandBuffers;
};

} // namespace huedra