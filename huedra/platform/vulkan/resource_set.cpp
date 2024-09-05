#include "resource_set.hpp"
#include "core/global.hpp"
#include "core/log.hpp"
#include "platform/vulkan/type_converter.hpp"

#include <set>

namespace huedra {

void VulkanResourceSet::init(Device& device, VulkanPipeline& pipeline, u32 setIndex)
{
    if (setIndex >= pipeline.getBuilder().getResources().size())
    {
        log(LogLevel::ERR, "Could not create resource set, requested setIndex does not exist in pipeline");
    }

    ResourceSet::init(setIndex);
    p_device = &device;
    p_pipeline = &pipeline;
    m_descriptors.resize(GraphicsManager::MAX_FRAMES_IN_FLIGHT);

    PipelineBuilder& builder = pipeline.getBuilder();
    std::vector<ResourceBinding> bindings = builder.getResources()[setIndex];
    m_descriptorTypes.reserve(bindings.size());

    std::multiset<VkDescriptorType> poolSizeSet{};
    for (auto& binding : bindings)
    {
        m_descriptorTypes.push_back(converter::convertResourceType(binding.resource));
        poolSizeSet.insert(m_descriptorTypes.back());
    }

    std::vector<VkDescriptorPoolSize> poolSizes;
    for (auto& set : poolSizeSet)
    {
        VkDescriptorPoolSize poolSize;
        poolSize.type = set;
        poolSize.descriptorCount = static_cast<u32>(poolSizeSet.count(set) * m_descriptors.size());
        poolSizes.push_back(poolSize);
    }

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<u32>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<u32>(m_descriptors.size());

    if (vkCreateDescriptorPool(p_device->getLogical(), &poolInfo, nullptr, &m_pool) != VK_SUCCESS)
    {
        log(LogLevel::ERR, "Failed to create descriptor pool!");
    }

    std::vector<VkDescriptorSetLayout> layouts(m_descriptors.size(), pipeline.getDescriptorLayout(setIndex));
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_pool;
    allocInfo.descriptorSetCount = static_cast<u32>(m_descriptors.size());
    allocInfo.pSetLayouts = layouts.data();

    if (vkAllocateDescriptorSets(p_device->getLogical(), &allocInfo, m_descriptors.data()) != VK_SUCCESS)
    {
        log(LogLevel::ERR, "Failed to allocate descriptor sets!");
    }
}

void VulkanResourceSet::cleanup() { vkDestroyDescriptorPool(p_device->getLogical(), m_pool, nullptr); }

void VulkanResourceSet::assignBuffer(Ref<Buffer> buffer, u32 binding)
{
    if (!buffer.valid())
    {
        log(LogLevel::WARNING, "Could not assign buffer to resource set, reference not valid");
        return;
    }

    p_device->waitIdle();

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.offset = 0;
    bufferInfo.buffer = static_cast<VulkanBuffer*>(buffer.get())->get();
    bufferInfo.range = VK_WHOLE_SIZE;

    std::vector<VkWriteDescriptorSet> descriptorWrites(m_descriptors.size());
    for (u64 i = 0; i < m_descriptors.size(); ++i)
    {
        descriptorWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[i].dstSet = m_descriptors[i];
        descriptorWrites[i].dstBinding = binding;
        descriptorWrites[i].dstArrayElement = 0;
        descriptorWrites[i].descriptorType = m_descriptorTypes[binding];
        descriptorWrites[i].descriptorCount = 1;
        descriptorWrites[i].pBufferInfo = &bufferInfo;
    }
    vkUpdateDescriptorSets(p_device->getLogical(), static_cast<u32>(descriptorWrites.size()), descriptorWrites.data(),
                           0, nullptr);
}

void VulkanResourceSet::assignTexture(Ref<Texture> texture, u32 binding)
{
    if (!texture.valid())
    {
        log(LogLevel::WARNING, "Could not assign texture to resource set, reference not valid");
        return;
    }

    p_device->waitIdle();

    VulkanTexture* vkTexture = static_cast<VulkanTexture*>(texture.get());
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = vkTexture->getType() == TextureType::COLOR
                                ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                                : VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    imageInfo.imageView = vkTexture->getView();
    imageInfo.sampler = vkTexture->getSampler();

    std::vector<VkWriteDescriptorSet> descriptorWrites(m_descriptors.size());
    for (u64 i = 0; i < m_descriptors.size(); ++i)
    {
        descriptorWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[i].dstSet = m_descriptors[i];
        descriptorWrites[i].dstBinding = binding;
        descriptorWrites[i].dstArrayElement = 0;
        descriptorWrites[i].descriptorType = m_descriptorTypes[binding];
        descriptorWrites[i].descriptorCount = 1;
        descriptorWrites[i].pImageInfo = &imageInfo;
    }
    vkUpdateDescriptorSets(p_device->getLogical(), static_cast<u32>(descriptorWrites.size()), descriptorWrites.data(),
                           0, nullptr);
}

VkDescriptorSet VulkanResourceSet::get() { return m_descriptors[Global::graphicsManager.getCurrentFrame()]; }

} // namespace huedra
