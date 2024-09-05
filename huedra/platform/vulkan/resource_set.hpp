#pragma once

#include "graphics/resource_set.hpp"
#include "platform/vulkan/buffer.hpp"
#include "platform/vulkan/device.hpp"
#include "platform/vulkan/pipeline.hpp"
#include "platform/vulkan/texture.hpp"

namespace huedra {

class VulkanResourceSet : public ResourceSet
{
public:
    VulkanResourceSet() = default;
    ~VulkanResourceSet() = default;

    void init(Device& device, VulkanPipeline& pipeline, u32 setIndex);
    void cleanup() override;

    void assignBuffer(Ref<Buffer> buffer, u32 binding) override;
    void assignTexture(Ref<Texture> texture, u32 binding) override;

    VkDescriptorSet get();
    VulkanPipeline* getPipeline() { return p_pipeline; }

private:
    Device* p_device;
    VulkanPipeline* p_pipeline;

    VkDescriptorPool m_pool;
    std::vector<VkDescriptorSet> m_descriptors;
    std::vector<VkDescriptorType> m_descriptorTypes;
};

} // namespace huedra
