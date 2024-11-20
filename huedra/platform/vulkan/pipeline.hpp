#pragma once

#include "graphics/pipeline_builder.hpp"
#include "platform/vulkan/device.hpp"

namespace huedra {

class VulkanPipeline
{
public:
    VulkanPipeline() = default;
    ~VulkanPipeline() = default;

    void initGraphics(const PipelineBuilder& pipelineBuilder, Device& device, VkRenderPass renderPass);
    void cleanup();

    PipelineBuilder& getBuilder() { return m_builder; }

    VkPipeline get() { return m_pipeline; };
    VkPipelineLayout getLayout() { return m_pipelineLayout; };
    std::vector<VkDescriptorSetLayout> getDescriptorLayouts() { return m_descriptorLayout; }
    VkDescriptorSetLayout getDescriptorLayout(size_t index) { return m_descriptorLayout[index]; }

private:
    void initLayout();

    VkShaderModule loadShader(const std::string& path);

    Device* p_device;
    PipelineBuilder m_builder;

    std::vector<VkDescriptorSetLayout> m_descriptorLayout;
    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_pipeline;
};

} // namespace huedra
