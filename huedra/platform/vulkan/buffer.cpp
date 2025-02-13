#include "buffer.hpp"
#include "core/global.hpp"
#include "core/log.hpp"

namespace huedra {

void VulkanBuffer::init(Device& device, BufferType type, u64 size, BufferUsageFlags usage,
                        VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, const void* data)
{
    Buffer::init(type, usage, size);
    p_device = &device;

    m_buffers.clear();
    m_memories.clear();
    m_mapped.clear();

    VkBufferCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.usage = usageFlags;
    createInfo.size = static_cast<VkDeviceSize>(size);

    if (type == BufferType::STATIC)
    {
        m_buffers.resize(1);
        m_memories.resize(1);
        m_mapped.resize(1, nullptr);
    }
    else
    {
        m_buffers.resize(GraphicsManager::MAX_FRAMES_IN_FLIGHT);
        m_memories.resize(GraphicsManager::MAX_FRAMES_IN_FLIGHT);
        m_mapped.resize(GraphicsManager::MAX_FRAMES_IN_FLIGHT, nullptr);
    }

    for (auto& buffer : m_buffers)
    {
        if (vkCreateBuffer(device.getLogical(), &createInfo, nullptr, &buffer) != VK_SUCCESS)
        {
            log(LogLevel::ERR, "Failed to create buffer!");
        }
    }

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(device.getLogical(), m_buffers[0], &memReqs);

    VkMemoryAllocateInfo memAlloc{};
    memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAlloc.allocationSize = memReqs.size;
    memAlloc.memoryTypeIndex = p_device->findMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags);

    VkMemoryAllocateFlagsInfoKHR allocFlagsInfo{};
    if (usageFlags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
    {
        allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
        allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        memAlloc.pNext = &allocFlagsInfo;
    }
    for (auto& memory : m_memories)
    {
        if (vkAllocateMemory(device.getLogical(), &memAlloc, nullptr, &memory) != VK_SUCCESS)
        {
            log(LogLevel::ERR, "Failed to allocate memory to buffer!");
        }
    }

    if (data != nullptr)
    {
        for (size_t i = 0; i < m_buffers.size(); ++i)
        {
            if (map(i))
            {
                log(LogLevel::ERR, "Failed to map buffer!");
            }
            memcpy(m_mapped[i], data, size);
            if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
            {
                VkMappedMemoryRange mappedRange = {};
                mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
                mappedRange.memory = m_memories[i];
                mappedRange.offset = 0;
                mappedRange.size = VK_WHOLE_SIZE;
                vkFlushMappedMemoryRanges(p_device->getLogical(), 1, &mappedRange);
            }
            unmap(i);
        }
    }

    for (size_t i = 0; i < m_buffers.size(); ++i)
    {
        if (vkBindBufferMemory(p_device->getLogical(), m_buffers[i], m_memories[i], 0) != VK_SUCCESS)
        {
            log(LogLevel::ERR, "Failed to bind buffer to memory!");
        }
        if (type == BufferType::DYNAMIC)
        {
            map(i);
        }
    }
}

void VulkanBuffer::cleanup()
{
    for (size_t i = 0; i < m_buffers.size(); ++i)
    {
        if (getType() == BufferType::DYNAMIC)
        {
            unmap(i);
        }
        vkFreeMemory(p_device->getLogical(), m_memories[i], nullptr);
        vkDestroyBuffer(p_device->getLogical(), m_buffers[i], nullptr);
    }
}

void VulkanBuffer::write(u64 size, void* data)
{
    bool isStatic = getType() == BufferType::STATIC;
    u32 index = isStatic ? 1 : global::graphicsManager.getCurrentFrame();

    if (isStatic)
    {
        map(index);
    }

    if (!m_mapped[index])
    {
        log(LogLevel::WARNING, "Could not write to buffer, memory is not mapped, therefore unaccessible");
        return;
    }
    memcpy(m_mapped[index], data, size);

    if (isStatic)
    {
        unmap(index);
    }
}

void VulkanBuffer::read(u64 size, void* data)
{
    bool isStatic = getType() == BufferType::STATIC;
    u32 index = isStatic ? 1 : global::graphicsManager.getCurrentFrame();

    if (isStatic)
    {
        map(index);
    }

    if (!m_mapped[index])
    {
        log(LogLevel::WARNING, "Could not read from buffer, memory is not mapped, therefore unaccessible");
        return;
    }
    memcpy(data, m_mapped[index], size);

    if (isStatic)
    {
        unmap(index);
    }
}

VkBuffer VulkanBuffer::get()
{
    return m_buffers[getType() == BufferType::STATIC ? 0 : global::graphicsManager.getCurrentFrame()];
}

bool VulkanBuffer::map(size_t index)
{
    return vkMapMemory(p_device->getLogical(), m_memories[index], 0, VK_WHOLE_SIZE, 0, &m_mapped[index]);
}

void VulkanBuffer::unmap(size_t index)
{
    if (m_mapped[index])
    {
        vkUnmapMemory(p_device->getLogical(), m_memories[index]);
        m_mapped[index] = nullptr;
    }
}

} // namespace huedra