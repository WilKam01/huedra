#include "command_buffer.hpp"
#include "core/log.hpp"

namespace huedra {

void CommandBuffer::init(Device& device, CommandPool& commandPool, u32 size)
{
    p_device = &device;
    p_commandPool = &commandPool;
    m_commandBuffers.resize(size);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool.get();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = size;

    if (vkAllocateCommandBuffers(device.getLogical(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS)
    {
        log(LogLevel::ERR, "Failed to allocate command buffers!");
    }
}

void CommandBuffer::cleanup()
{
    vkFreeCommandBuffers(p_device->getLogical(), p_commandPool->get(), static_cast<u32>(m_commandBuffers.size()),
                         m_commandBuffers.data());
    m_commandBuffers.clear();
}

void CommandBuffer::begin(size_t index)
{
    vkResetCommandBuffer(m_commandBuffers[index], 0);
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(m_commandBuffers[index], &beginInfo) != VK_SUCCESS)
    {
        log(LogLevel::ERR, "Failed to begin recording command buffer!\nIndex: %d", index);
    }
}

void CommandBuffer::end(size_t index)
{
    if (vkEndCommandBuffer(m_commandBuffers[index]) != VK_SUCCESS)
    {
        log(LogLevel::ERR, "Failed to end command buffer!");
    }
}

} // namespace huedra