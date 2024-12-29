#pragma once

#include "graphics/pipeline_data.hpp"
#include "platform/vulkan/device.hpp"

namespace huedra {

class VulkanBuffer
{
public:
    VulkanBuffer() = default;
    ~VulkanBuffer() = default;

    void init(Device& device, BufferType type, u64 size, VkBufferUsageFlags usageFlags,
              VkMemoryPropertyFlags memoryPropertyFlags, void* data = nullptr);
    void cleanup();

    void write(u64 size, void* data);
    void read(u64 size, void* data);

    VkBuffer get();

private:
    bool map(size_t index);
    void unmap(size_t index);

    Device* p_device;
    BufferType m_type{BufferType::STATIC};
    std::vector<VkBuffer> m_buffers;
    std::vector<VkDeviceMemory> m_memories;
    std::vector<void*> m_mapped{};
};

} // namespace huedra