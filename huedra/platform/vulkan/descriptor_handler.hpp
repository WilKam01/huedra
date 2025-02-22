#pragma once

#include "platform/vulkan/buffer.hpp"
#include "platform/vulkan/device.hpp"
#include "platform/vulkan/render_pass.hpp"

namespace huedra {

class DescriptorHandler
{
public:
    DescriptorHandler() = default;
    virtual ~DescriptorHandler() = default;

    DescriptorHandler(const DescriptorHandler& rhs) = default;
    DescriptorHandler& operator=(const DescriptorHandler& rhs) = default;
    DescriptorHandler(DescriptorHandler&& rhs) = default;
    DescriptorHandler& operator=(DescriptorHandler&& rhs) = default;

    void init(Device& device, VulkanRenderPass& renderPass, VkDescriptorPool descriptorPool,
              const std::vector<std::vector<VkDescriptorType>>& bindingTypes);
    void cleanup();

    void updateSetInstance();
    void resetSetInstance();
    void bindSets(VkCommandBuffer commandBuffer);

    void writeBuffer(VulkanBuffer& buffer, u32 set, u32 binding);
    void writeTexture(VulkanTexture& texture, VkSampler sampler, u32 set, u32 binding);

private:
    VkDescriptorSet createDescriptorSet(u32 set);

    Device* m_device{nullptr};
    VulkanRenderPass* m_renderPass{nullptr};
    VkDescriptorPool m_pool{nullptr};
    std::vector<VkDescriptorSetLayout> m_layouts;
    struct SetInfo
    {
        std::vector<VkDescriptorSet> instances;
        std::vector<VkDescriptorType> bindingTypes;
        u64 curIndex{0};
    };
    std::vector<SetInfo> m_sets;
    bool m_updatedSinceLastUpdate{false};
};

} // namespace huedra