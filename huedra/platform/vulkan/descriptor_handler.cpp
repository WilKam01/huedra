#include "descriptor_handler.hpp"

#include "core/log.hpp"
#include "platform/vulkan/type_converter.hpp"

namespace huedra {

void DescriptorHandler::init(Device& device, VulkanRenderPass& renderPass, VkDescriptorPool descriptorPool,
                             const std::vector<std::vector<VkDescriptorType>>& bindingTypes)
{
    m_device = &device;
    m_renderPass = &renderPass;
    m_pool = descriptorPool;
    m_layouts = renderPass.getPipeline().getDescriptorLayouts();

    m_sets.resize(bindingTypes.size());
    for (u64 i = 0; i < m_sets.size(); ++i)
    {
        m_sets[i].bindingTypes = bindingTypes[i];
    }
}

void DescriptorHandler::cleanup() { m_sets.clear(); }

void DescriptorHandler::updateSetInstance()
{
    if (m_updatedSinceLastUpdate)
    {
        for (auto& set : m_sets)
        {
            if (set.curIndex < set.instances.size())
            {
                ++set.curIndex;
            }
        }
    }
    m_updatedSinceLastUpdate = false;
}

void DescriptorHandler::resetSetInstance()
{
    for (auto& set : m_sets)
    {
        set.curIndex = 0;
    }
    m_updatedSinceLastUpdate = false;
}

void DescriptorHandler::bindSets(VkCommandBuffer commandBuffer)
{
    if (m_sets.empty() || !m_updatedSinceLastUpdate)
    {
        return;
    }

    std::vector<VkDescriptorSet> descriptors(m_sets.size());
    for (u64 i = 0; i < m_sets.size(); ++i)
    {
        if (m_sets[i].instances.empty() || m_sets[i].curIndex >= m_sets[i].instances.size())
        {
            log(LogLevel::WARNING, "Could not bind sets, set {} has not been set", i);
            return;
        }
        descriptors[i] = m_sets[i].instances[m_sets[i].curIndex];
    }

    vkCmdBindDescriptorSets(commandBuffer, converter::convertPipelineType(m_renderPass->getPipelineType()),
                            m_renderPass->getPipeline().getLayout(), 0, static_cast<u32>(descriptors.size()),
                            descriptors.data(), 0, nullptr);
}

void DescriptorHandler::writeBuffer(VulkanBuffer& buffer, u32 set, u32 binding)
{
    if (set >= m_sets.size())
    {
        log(LogLevel::WARNING, "Could not write buffer, set {} out of bounds ({} defined)", set, m_sets.size());
        return;
    }

    if (binding >= m_sets[set].bindingTypes.size())
    {
        log(LogLevel::WARNING, "Could not write buffer, binding {} out of bounds ({} defined)", binding,
            m_sets[set].bindingTypes.size());
        return;
    }

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.offset = 0;
    bufferInfo.buffer = buffer.get();
    bufferInfo.range = VK_WHOLE_SIZE;

    if (m_sets[set].curIndex == m_sets[set].instances.size())
    {
        m_sets[set].instances.push_back(createDescriptorSet(set));
    }

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = m_sets[set].instances[m_sets[set].curIndex];
    descriptorWrite.dstBinding = binding;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = m_sets[set].bindingTypes[binding];
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(m_device->getLogical(), 1, &descriptorWrite, 0, nullptr);
    m_updatedSinceLastUpdate = true;
}

void DescriptorHandler::writeTexture(VulkanTexture& texture, u32 set, u32 binding)
{
    if (set >= m_sets.size())
    {
        log(LogLevel::WARNING, "Could not write texture, set {} out of bounds ({} defined)", set, m_sets.size());
        return;
    }

    if (binding >= m_sets[set].bindingTypes.size())
    {
        log(LogLevel::WARNING, "Could not write texture, binding {} out of bounds ({} defined)", binding,
            m_sets[set].bindingTypes.size());
        return;
    }

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = texture.getLayout();
    imageInfo.imageView = texture.getView();

    if (m_sets[set].curIndex == m_sets[set].instances.size())
    {
        m_sets[set].instances.push_back(createDescriptorSet(set));
    }

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = m_sets[set].instances[m_sets[set].curIndex];
    descriptorWrite.dstBinding = binding;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = m_sets[set].bindingTypes[binding];
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(m_device->getLogical(), 1, &descriptorWrite, 0, nullptr);
    m_updatedSinceLastUpdate = true;
}

void DescriptorHandler::writeSampler(VkSampler sampler, u32 set, u32 binding)
{
    if (set >= m_sets.size())
    {
        log(LogLevel::WARNING, "Could not write sampler, set {} out of bounds ({} defined)", set, m_sets.size());
        return;
    }

    if (binding >= m_sets[set].bindingTypes.size())
    {
        log(LogLevel::WARNING, "Could not write sampler, binding {} out of bounds ({} defined)", binding,
            m_sets[set].bindingTypes.size());
        return;
    }

    VkDescriptorImageInfo imageInfo{};
    imageInfo.sampler = sampler;

    if (m_sets[set].curIndex == m_sets[set].instances.size())
    {
        m_sets[set].instances.push_back(createDescriptorSet(set));
    }

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = m_sets[set].instances[m_sets[set].curIndex];
    descriptorWrite.dstBinding = binding;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = m_sets[set].bindingTypes[binding];
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(m_device->getLogical(), 1, &descriptorWrite, 0, nullptr);
    m_updatedSinceLastUpdate = true;
}

VkDescriptorSet DescriptorHandler::createDescriptorSet(u32 set)
{
    VkDescriptorSet descriptor{nullptr};
    VkDescriptorSetLayout layout = m_renderPass->getPipeline().getDescriptorLayout(set);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout;

    if (vkAllocateDescriptorSets(m_device->getLogical(), &allocInfo, &descriptor) != VK_SUCCESS)
    {
        log(LogLevel::ERR, "Failed to allocate descriptor set!");
    }

    return descriptor;
}

} // namespace huedra
