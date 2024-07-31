#pragma once

#include "graphics/pipeline_data.hpp"
#include "platform/vulkan/config.hpp"

namespace huedra::converter {

// HU -> VK
VkPipelineBindPoint convertPipelineType(PipelineType type);
VkShaderStageFlagBits convertShaderStage(PipelineType type, ShaderStageFlags shaderStage);
VkDescriptorType convertResourceType(ResourceType resource);
VkFormat convertDataFormat(GraphicsDataFormat format);
VkVertexInputRate convertVertexInputRate(VertexInputRate inputRate);

// VK -> HU
GraphicsDataFormat convertVkFormat(VkFormat format);

} // namespace huedra::converter