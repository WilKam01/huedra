#include "command_pool.hpp"
#include "core/log.hpp"

namespace huedra {

void CommandPool::init(Device& device, VkPipelineBindPoint pipeline)
{
    m_device = &device;
    m_pipeline = pipeline;

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = m_pipeline == VK_PIPELINE_BIND_POINT_COMPUTE ? device.getComputeQueueFamilyIndex()
                                                                             : device.getGraphicsQueueFamilyIndex();

    if (vkCreateCommandPool(device.getLogical(), &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS)
    {
        log(LogLevel::ERR, "Failed to create command pool!");
    }
}

void CommandPool::cleanup() { vkDestroyCommandPool(m_device->getLogical(), m_commandPool, nullptr); }

VkCommandBuffer CommandPool::beginSingleTimeCommand()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer{nullptr};
    vkAllocateCommandBuffers(m_device->getLogical(), &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    return commandBuffer;
}

void CommandPool::endSingleTimeCommand(VkCommandBuffer buffer)
{
    vkEndCommandBuffer(buffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &buffer;

    if (m_pipeline == VK_PIPELINE_BIND_POINT_COMPUTE)
    {
        vkQueueSubmit(m_device->getComputeQueue(), 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_device->getComputeQueue());
    }
    else
    {
        vkQueueSubmit(m_device->getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_device->getGraphicsQueue());
    }

    vkFreeCommandBuffers(m_device->getLogical(), m_commandPool, 1, &buffer);
}

} // namespace huedra
