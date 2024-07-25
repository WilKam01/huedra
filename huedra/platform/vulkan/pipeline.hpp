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
    VkPipelineLayout getLayout() { return m_layout; };

private:
    void initLayout();
    // TODO: Move byte reading to asset/io manager
    VkShaderModule loadShader(const std::string& path);

    Device* p_device;
    VkPipeline m_pipeline;
    VkPipelineLayout m_layout;
};

} // namespace huedra
