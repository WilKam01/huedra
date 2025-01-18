#pragma once

#include "graphics/buffer.hpp"
#include "platform/vulkan/device.hpp"

namespace huedra {

class VulkanBuffer : public Buffer
{
public:
    VulkanBuffer() = default;
    ~VulkanBuffer() = default;

    void init(Device& device, BufferType type, u64 size, BufferUsageFlags usage, VkBufferUsageFlags usageFlags,
              VkMemoryPropertyFlags memoryPropertyFlags, void* data = nullptr);
    void cleanup();

    void write(u64 size, void* data) override;
    void read(u64 size, void* data) override;

    VkBuffer get();

private:
    bool map(size_t index);
    void unmap(size_t index);

    Device* p_device;
    std::vector<VkBuffer> m_buffers;
    std::vector<VkDeviceMemory> m_memories;
    std::vector<void*> m_mapped{};
};

} // namespace huedra