#pragma once

#include "graphics/buffer.hpp"
#include "platform/vulkan/device.hpp"

namespace huedra {

class VulkanBuffer : public Buffer
{
public:
    VulkanBuffer() = default;
    ~VulkanBuffer() override = default;

    VulkanBuffer(const VulkanBuffer& rhs) = default;
    VulkanBuffer& operator=(const VulkanBuffer& rhs) = default;
    VulkanBuffer(VulkanBuffer&& rhs) = default;
    VulkanBuffer& operator=(VulkanBuffer&& rhs) = default;

    void init(Device& device, BufferType type, u64 size, BufferUsageFlags usage, VkBufferUsageFlags usageFlags,
              VkMemoryPropertyFlags memoryPropertyFlags, const void* data = nullptr);
    void cleanup();

    void write(u64 size, void* data) override;
    void read(u64 size, void* data) override;

    VkBuffer get();

private:
    bool map(u64 index);
    void unmap(u64 index);

    Device* m_device{nullptr};
    std::vector<VkBuffer> m_buffers;
    std::vector<VkDeviceMemory> m_memories;
    std::vector<void*> m_mapped;
};

} // namespace huedra