#pragma once

#include "graphics/pipeline_builder.hpp"
#include "platform/vulkan/device.hpp"

namespace huedra {

class VulkanPipeline
{
public:
    VulkanPipeline() = default;
    ~VulkanPipeline() = default;

    VulkanPipeline(const VulkanPipeline& rhs) = default;
    VulkanPipeline& operator=(const VulkanPipeline& rhs) = default;
    VulkanPipeline(VulkanPipeline&& rhs) = default;
    VulkanPipeline& operator=(VulkanPipeline&& rhs) = default;

    void initGraphics(const PipelineBuilder& pipelineBuilder, Device& device, VkRenderPass renderPass, u32 targetCount);
    void initCompute(const PipelineBuilder& pipelineBuilder, Device& device);
    void cleanup();

    PipelineBuilder& getBuilder() { return m_builder; }

    VkPipeline get() { return m_pipeline; };
    VkPipelineLayout getLayout() { return m_pipelineLayout; };
    std::vector<VkDescriptorSetLayout> getDescriptorLayouts() { return m_descriptorLayout; }
    VkDescriptorSetLayout getDescriptorLayout(u64 index) { return m_descriptorLayout[index]; }

private:
    void initLayout();

    VkShaderModule loadShader(const ShaderModule& shader);

    Device* m_device{nullptr};
    PipelineBuilder m_builder;

    std::vector<VkDescriptorSetLayout> m_descriptorLayout;
    VkPipelineLayout m_pipelineLayout{nullptr};
    VkPipeline m_pipeline{nullptr};
};

} // namespace huedra
