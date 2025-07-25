#include "render_context.hpp"
#include "core/log.hpp"
#include "graphics/buffer.hpp"
#include "graphics/pipeline_data.hpp"
#include "platform/vulkan/context.hpp"
#include "platform/vulkan/type_converter.hpp"

namespace huedra {

void VulkanRenderContext::init(VkCommandBuffer commandBuffer, VulkanContext* context, VulkanRenderPass* renderPass,
                               DescriptorHandler& descriptorHandler)
{
    m_commandBuffer = commandBuffer;
    m_context = context;
    m_renderPass = renderPass;
    m_descriptorHandler = &descriptorHandler;
    m_boundIndexBuffer = false;
    m_descriptorHandler->resetSetInstance();
}

void VulkanRenderContext::bindVertexBuffers(std::vector<Ref<Buffer>> buffers)
{
    if (m_renderPass->getPipelineType() != PipelineType::GRAPHICS)
    {
        log(LogLevel::WARNING, "Could not bind vertex buffers, not using a graphics pipeline");
        return;
    }

    std::vector<VkBuffer> vkBuffers(buffers.size());
    std::vector<VkDeviceSize> offsets(buffers.size(), 0);
    for (u64 i = 0; i < buffers.size(); ++i)
    {
        if (!buffers[i].valid())
        {
            log(LogLevel::WARNING, "Could not bind vertex buffer: {}. Not valid", i);
            return;
        }
        if ((buffers[i]->getBufferUsage() & HU_BUFFER_USAGE_VERTEX_BUFFER) == 0)
        {
            log(LogLevel::WARNING, "Could not bind vertex buffer: {}. Buffer usage flag vertex buffer not set", i);
            return;
        }
        vkBuffers[i] = static_cast<VulkanBuffer*>(buffers[i].get())->get();
    }

    vkCmdBindVertexBuffers(m_commandBuffer, 0, static_cast<u32>(vkBuffers.size()), vkBuffers.data(), offsets.data());
}

void VulkanRenderContext::bindIndexBuffer(Ref<Buffer> buffer)
{
    if (m_renderPass->getPipelineType() != PipelineType::GRAPHICS)
    {
        log(LogLevel::WARNING, "Could not bind index buffer, not using a graphics pipeline");
        return;
    }

    if (!buffer.valid())
    {
        log(LogLevel::WARNING, "Could not bind index buffer. Not valid");
        return;
    }

    if ((buffer->getBufferUsage() & HU_BUFFER_USAGE_INDEX_BUFFER) == 0)
    {
        log(LogLevel::WARNING, "Could not bind index buffer. Buffer usage flag index buffer not set");
        return;
    }

    VkBuffer vkBuffer = static_cast<VulkanBuffer*>(buffer.get())->get();
    vkCmdBindIndexBuffer(m_commandBuffer, vkBuffer, 0, VK_INDEX_TYPE_UINT32);
    m_boundIndexBuffer = true;
}

void VulkanRenderContext::bindBuffer(Ref<Buffer> buffer, std::string_view name)
{
    if (!buffer.valid())
    {
        log(LogLevel::WARNING, "Could not bind buffer, reference invalid");
        return;
    }

    std::optional<ResourcePosition> resource = m_renderPass->getPipeline().getShaderModule().getResource(name);
    if (!resource.has_value())
    {
        log(LogLevel::WARNING, "Could not bind buffer, no resource named \"{}\"", name);
        return;
    }

    if (resource.value().info.type != ResourceType::CONSTANT_BUFFER &&
        resource.value().info.type != ResourceType::STRUCTURED_BUFFER)
    {
        log(LogLevel::WARNING, "Could not bind buffer, \"{}\" is a {}", name,
            ResourceTypeNames[static_cast<u32>(resource.value().info.type)]);
        return;
    }

    m_descriptorHandler->writeBuffer(*static_cast<VulkanBuffer*>(buffer.get()), resource.value().set,
                                     resource.value().binding);
}

void VulkanRenderContext::bindTexture(Ref<Texture> texture, std::string_view name)
{
    if (!texture.valid())
    {
        log(LogLevel::WARNING, "Could not bind texture, reference invalid");
        return;
    }

    std::optional<ResourcePosition> resource = m_renderPass->getPipeline().getShaderModule().getResource(name);
    if (!resource.has_value())
    {
        log(LogLevel::WARNING, "Could not bind texture, no resource named \"{}\"", name);
        return;
    }

    if (resource.value().info.type != ResourceType::TEXTURE && resource.value().info.type != ResourceType::RW_TEXTURE)
    {
        log(LogLevel::WARNING, "Could not bind texture, \"{}\" is a {}", name,
            ResourceTypeNames[static_cast<u32>(resource.value().info.type)]);
        return;
    }

    m_descriptorHandler->writeTexture(*static_cast<VulkanTexture*>(texture.get()), resource.value().set,
                                      resource.value().binding);
}

void VulkanRenderContext::bindSampler(const SamplerSettings& sampler, std::string_view name)
{
    std::optional<ResourcePosition> resource = m_renderPass->getPipeline().getShaderModule().getResource(name);
    if (!resource.has_value())
    {
        log(LogLevel::WARNING, "Could not bind sampler, no resource named \"{}\"", name);
        return;
    }

    if (resource.value().info.type != ResourceType::SAMPLER)
    {
        log(LogLevel::WARNING, "Could not bind sampler, \"{}\" is a {}", name,
            ResourceTypeNames[static_cast<u32>(resource.value().info.type)]);
        return;
    }

    m_descriptorHandler->writeSampler(m_context->getSampler(sampler), resource.value().set, resource.value().binding);
}

void VulkanRenderContext::setParameter(void* data, u32 size, std::string_view name)
{
    std::optional<ParameterBinding> parameter = m_renderPass->getPipeline().getShaderModule().getParameter(name);
    if (!parameter.has_value())
    {
        log(LogLevel::WARNING, "Could not set parameter, no parameter named \"{}\"", name);
        return;
    }

    if (parameter.value().size != size)
    {
        log(LogLevel::WARNING, "Could not set parameter \"{}\", got size {}, expected {}", name, size,
            parameter.value().size);
        return;
    }

    vkCmdPushConstants(m_commandBuffer, m_renderPass->getPipeline().getLayout(),
                       converter::convertShaderStage(m_renderPass->getPipelineType(), parameter.value().shaderStage),
                       parameter.value().offset, size, data);
}

void VulkanRenderContext::draw(u32 vertexCount, u32 instanceCount, u32 vertexOffset, u32 instanceOffset)
{
    if (m_renderPass->getPipelineType() != PipelineType::GRAPHICS)
    {
        log(LogLevel::WARNING, "Could not execute draw command, not using a graphics pipeline");
        return;
    }

    m_descriptorHandler->bindSets(m_commandBuffer);
    vkCmdDraw(m_commandBuffer, vertexCount, instanceCount, vertexOffset, instanceOffset);
    m_descriptorHandler->updateSetInstance();
}

void VulkanRenderContext::drawIndexed(u32 indexCount, u32 instanceCount, u32 indexOffset, u32 instanceOffset)
{
    if (m_renderPass->getPipelineType() != PipelineType::GRAPHICS)
    {
        log(LogLevel::WARNING, "Could not execute draw call, not using a graphics pipeline");
        return;
    }

    if (!m_boundIndexBuffer)
    {
        log(LogLevel::WARNING, "Could not execute drawIndexed command, no index buffer has been bound");
        return;
    }

    m_descriptorHandler->bindSets(m_commandBuffer);
    vkCmdDrawIndexed(m_commandBuffer, indexCount, instanceCount, indexOffset, 0, instanceOffset);
    m_descriptorHandler->updateSetInstance();
}

void VulkanRenderContext::dispatchGroups(u32 groupX, u32 groupY, u32 groupZ)
{
    if (m_renderPass->getPipelineType() != PipelineType::COMPUTE)
    {
        log(LogLevel::WARNING, "Could not execute dispatchGroups call, not using a compute pipeline");
        return;
    }

    m_descriptorHandler->bindSets(m_commandBuffer);
    vkCmdDispatch(m_commandBuffer, groupX, groupY, groupZ);
    m_descriptorHandler->updateSetInstance();
}

void VulkanRenderContext::dispatch(u32 x, u32 y, u32 z)
{
    if (m_renderPass->getPipelineType() != PipelineType::COMPUTE)
    {
        log(LogLevel::WARNING, "Could not execute dispatch call, not using a compute pipeline");
        return;
    }

    m_descriptorHandler->bindSets(m_commandBuffer);
    uvec3 computeThreadsPerGroup = m_renderPass->getPipeline().getShaderModule().getComputeThreadsPerGroup();
    vkCmdDispatch(m_commandBuffer, (x + computeThreadsPerGroup.x - 1) / computeThreadsPerGroup.x,
                  (y + computeThreadsPerGroup.y - 1) / computeThreadsPerGroup.y,
                  (z + computeThreadsPerGroup.z - 1) / computeThreadsPerGroup.z);
    m_descriptorHandler->updateSetInstance();
}

} // namespace huedra