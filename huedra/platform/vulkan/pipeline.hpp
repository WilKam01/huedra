#pragma once

#include "graphics/pipeline.hpp"
#include "platform/vulkan/device.hpp"

namespace huedra {

class VulkanPipeline : public Pipeline
{
public:
    VulkanPipeline() = default;
    ~VulkanPipeline() = default;

    void initGraphics(const PipelineBuilder& pipelineBuilder, Device& device, VkRenderPass renderPass);
    void cleanup() override;

    VkPipeline get() { return m_pipeline; };
    VkPipelineLayout getLayout() { return m_pipelineLayout; };

    VkShaderStageFlagBits convertShaderStage(ShaderStageFlags shaderStage);

private:
    void initLayout();
    VkDescriptorType convertResourceType(ResourceType resource);
    VkVertexInputRate convertVertexInputRate(VertexInputRate inputRate);
    VkFormat convertDataFormat(GraphicsDataFormat format);

    // TODO: Move byte reading to asset/io manager
    VkShaderModule loadShader(const std::string& path);

    Device* p_device;

    std::vector<VkDescriptorSetLayout> m_descriptorLayout;
    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_pipeline;
};

} // namespace huedra
