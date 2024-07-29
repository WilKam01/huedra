#include "resource_set.hpp"
#include "core/global.hpp"
#include "core/log.hpp"

#include <set>

namespace huedra {

void VulkanResourceSet::init(Device& device, VulkanPipeline& pipeline, u32 setIndex)
{
    ResourceSet::init(&pipeline, setIndex);
    p_device = &device;
    m_descriptors.resize(GraphicsManager::MAX_FRAMES_IN_FLIGHT);

    PipelineBuilder& builder = pipeline.getBuilder();
    std::vector<ResourceBinding> bindings = builder.getResources()[setIndex];
    m_descriptorTypes.reserve(bindings.size());

    std::multiset<VkDescriptorType> poolSizeSet{};
    for (auto& binding : bindings)
    {
        m_descriptorTypes.push_back(pipeline.convertResourceType(binding.resource));
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

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.offset = 0;
    bufferInfo.buffer = static_cast<VulkanBuffer*>(buffer.get())->get();
    bufferInfo.range = VK_WHOLE_SIZE;

    VkWriteDescriptorSet descriptorWrites{};
    descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites.dstSet = m_descriptors[Global::graphicsManager.getCurrentFrame()];
    descriptorWrites.dstBinding = binding;
    descriptorWrites.dstArrayElement = 0;
    descriptorWrites.descriptorType = m_descriptorTypes[binding];
    descriptorWrites.descriptorCount = 1;
    descriptorWrites.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(p_device->getLogical(), 1, &descriptorWrites, 0, nullptr);
}

VkDescriptorSet VulkanResourceSet::get() { return m_descriptors[Global::graphicsManager.getCurrentFrame()]; }

} // namespace huedra
