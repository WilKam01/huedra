#include "render_context.hpp"
#include "core/log.hpp"
#include "platform/vulkan/type_converter.hpp"

namespace huedra {

void VulkanRenderContext::init(VkCommandBuffer commandBuffer, VulkanPipeline* pipeline)
{
    m_commandBuffer = commandBuffer;
    p_pipeline = pipeline;
    m_boundVertexBuffer = false;
    m_boundIndexBuffer = false;
    vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, p_pipeline->get());
}

void VulkanRenderContext::bindVertexBuffers(std::vector<Ref<Buffer>> buffers)
{
    if (p_pipeline->getBuilder().getType() != PipelineType::GRAPHICS)
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
    if (p_pipeline->getBuilder().getType() != PipelineType::GRAPHICS)
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

void VulkanRenderContext::bindResourceSets(std::vector<Ref<ResourceSet>> resourceSets)
{
    if (resourceSets.size() > p_pipeline->getBuilder().getResources().size())
    {
        log(LogLevel::WARNING, "Could not bind resource sets, pipeline supports %d sets but %d was provided",
            p_pipeline->getBuilder().getResources().size(), resourceSets.size());
    }

    for (size_t i = 0; i < resourceSets.size(); ++i)
    {
        if (!resourceSets[i].valid() || !resourceSets[i].get()->getPipeline().valid())
        {
            log(LogLevel::WARNING, "Could not bind resource set: %d. Not valid", i);
            return;
        }

        if (static_cast<Pipeline*>(p_pipeline) != resourceSets[i].get()->getPipeline().get())
        {
            log(LogLevel::WARNING, "Could not bind resource set: %d. Using different pipelines", i);
            return;
        }
    }

    u32 setOffset = resourceSets[0].get()->getSetIndex();
    std::vector<VkDescriptorSet> descriptors(resourceSets.size());
    for (size_t i = 0; i < resourceSets.size(); ++i)
    {
        descriptors[i] = static_cast<VulkanResourceSet*>(resourceSets[i].get())->get();
    }

    vkCmdBindDescriptorSets(m_commandBuffer, converter::convertPipelineType(p_pipeline->getBuilder().getType()),
                            p_pipeline->getLayout(), setOffset, static_cast<u32>(descriptors.size()),
                            descriptors.data(), 0, nullptr);
}

void VulkanRenderContext::bindResourceSet(Ref<ResourceSet> resourceSet)
{
    if (!resourceSet.valid() || !resourceSet.get()->getPipeline().valid())
    {
        log(LogLevel::WARNING, "Could not bind resource set. Not valid");
        return;
    }

    if (static_cast<Pipeline*>(p_pipeline) != resourceSet.get()->getPipeline().get())
    {
        log(LogLevel::WARNING, "Could not bind resource set. Using different pipelines");
        return;
    }

    VulkanResourceSet* set = static_cast<VulkanResourceSet*>(resourceSet.get());
    VkDescriptorSet descriptorSet = set->get();

    vkCmdBindDescriptorSets(m_commandBuffer, converter::convertPipelineType(p_pipeline->getBuilder().getType()),
                            p_pipeline->getLayout(), set->getSetIndex(), 1, &descriptorSet, 0, nullptr);
}

void VulkanRenderContext::pushConstants(ShaderStageFlags shaderStage, u32 size, void* data)
{
    vkCmdPushConstants(m_commandBuffer, p_pipeline->getLayout(),
                       converter::convertShaderStage(p_pipeline->getBuilder().getType(), shaderStage), 0, size, data);
}

void VulkanRenderContext::draw(u32 vertexCount, u32 instanceCount, u32 vertexOffset, u32 instanceOffset)
{
    if (p_pipeline->getBuilder().getType() != PipelineType::GRAPHICS)
    {
        log(LogLevel::WARNING, "Could not execute draw command, not using a graphics pipeline");
        return;
    }

    if (!m_boundVertexBuffer)
    {
        log(LogLevel::WARNING, "Could not execute draw command, no vertex buffer has been bound");
        return;
    }

    vkCmdDraw(m_commandBuffer, vertexCount, instanceCount, vertexOffset, instanceOffset);
}

void VulkanRenderContext::drawIndexed(u32 indexCount, u32 instanceCount, u32 indexOffset, u32 instanceOffset)
{
    if (p_pipeline->getBuilder().getType() != PipelineType::GRAPHICS)
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

    vkCmdDrawIndexed(m_commandBuffer, indexCount, instanceCount, indexOffset, 0, instanceOffset);
}

} // namespace huedra