#include "render_context.hpp"
#include "core/log.hpp"
#include "graphics/buffer.hpp"
#include "platform/vulkan/context.hpp"
#include "platform/vulkan/type_converter.hpp"

namespace huedra {

void VulkanRenderContext::init(VkCommandBuffer commandBuffer, VulkanContext* context, VulkanRenderPass* renderPass,
                               DescriptorHandler& descriptorHandler)
{
    m_commandBuffer = commandBuffer;
    p_context = context;
    p_renderPass = renderPass;
    p_descriptorHandler = &descriptorHandler;
    m_boundVertexBuffer = false;
    m_boundIndexBuffer = false;
    p_descriptorHandler->resetSetInstance();
}

void VulkanRenderContext::bindVertexBuffers(std::vector<Ref<Buffer>> buffers)
{
    if (p_renderPass->getPipelineType() != PipelineType::GRAPHICS)
    {
        log(LogLevel::WARNING, "Could not execute draw command, not using a graphics pipeline");
        return;
    }

    std::vector<VkBuffer> vkBuffers(buffers.size());
    std::vector<VkDeviceSize> offsets(buffers.size(), 0);
    for (size_t i = 0; i < buffers.size(); ++i)
    {
        if (!buffers[i].valid())
        {
            log(LogLevel::ERR, "Could not bind vertex buffer: %d. Not valid", i);
        }
        vkBuffers[i] = static_cast<VulkanBuffer*>(buffers[i].get())->get();
    }

    vkCmdBindVertexBuffers(m_commandBuffer, 0, static_cast<u32>(vkBuffers.size()), vkBuffers.data(), offsets.data());
    m_boundVertexBuffer = true;
}

void VulkanRenderContext::bindIndexBuffer(Ref<Buffer> buffer)
{
    if (p_renderPass->getPipelineType() != PipelineType::GRAPHICS)
    {
        log(LogLevel::WARNING, "Could not execute draw command, not using a graphics pipeline");
        return;
    }

    if (!buffer.valid())
    {
        log(LogLevel::ERR, "Could not bind index buffer. Not valid");
    }

    VkBuffer vkBuffer = static_cast<VulkanBuffer*>(buffer.get())->get();
    vkCmdBindIndexBuffer(m_commandBuffer, vkBuffer, 0, VK_INDEX_TYPE_UINT32);
    m_boundIndexBuffer = true;
}

void VulkanRenderContext::bindBuffer(Ref<Buffer> buffer, u32 set, u32 binding)
{
    if (!buffer.valid())
    {
        log(LogLevel::WARNING, "Could not bind buffer, reference invalid");
    }

    p_descriptorHandler->writeBuffer(*static_cast<VulkanBuffer*>(buffer.get()), set, binding);
}

void VulkanRenderContext::bindTexture(Ref<Texture> texture, u32 set, u32 binding)
{
    if (!texture.valid())
    {
        log(LogLevel::WARNING, "Could not bind texture, reference invalid");
    }

    p_descriptorHandler->writeTexture(*static_cast<VulkanTexture*>(texture.get()), set, binding);
}

void VulkanRenderContext::pushConstants(ShaderStageFlags shaderStage, u32 size, void* data)
{
    vkCmdPushConstants(m_commandBuffer, p_renderPass->getPipeline().getLayout(),
                       converter::convertShaderStage(p_renderPass->getPipelineType(), shaderStage), 0, size, data);
}

void VulkanRenderContext::draw(u32 vertexCount, u32 instanceCount, u32 vertexOffset, u32 instanceOffset)
{
    if (p_renderPass->getPipelineType() != PipelineType::GRAPHICS)
    {
        log(LogLevel::WARNING, "Could not execute draw command, not using a graphics pipeline");
        return;
    }

    if (!m_boundVertexBuffer)
    {
        log(LogLevel::WARNING, "Could not execute draw command, no vertex buffer has been bound");
        return;
    }

    p_descriptorHandler->bindSets(m_commandBuffer);
    vkCmdDraw(m_commandBuffer, vertexCount, instanceCount, vertexOffset, instanceOffset);
    p_descriptorHandler->updateSetInstance();
}

void VulkanRenderContext::drawIndexed(u32 indexCount, u32 instanceCount, u32 indexOffset, u32 instanceOffset)
{
    if (p_renderPass->getPipelineType() != PipelineType::GRAPHICS)
    {
        log(LogLevel::WARNING, "Could not execute draw command, not using a graphics pipeline");
        return;
    }

    if (!m_boundVertexBuffer)
    {
        log(LogLevel::WARNING, "Could not execute drawIndexed command, no vertex buffer has been bound");
        return;
    }

    if (!m_boundIndexBuffer)
    {
        log(LogLevel::WARNING, "Could not execute drawIndexed command, no index buffer has been bound");
        return;
    }

    p_descriptorHandler->bindSets(m_commandBuffer);
    vkCmdDrawIndexed(m_commandBuffer, indexCount, instanceCount, indexOffset, 0, instanceOffset);
    p_descriptorHandler->updateSetInstance();
}

} // namespace huedra