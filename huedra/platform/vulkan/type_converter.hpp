#pragma once

#include "graphics/pipeline_data.hpp"
#include "platform/vulkan/config.hpp"

namespace huedra::converter {

// HU -> VK
VkPipelineBindPoint convertPipelineType(PipelineType type);

// HU -> VK
VkShaderStageFlagBits convertShaderStage(PipelineType type, ShaderStage shaderStage);

// HU -> VK
VkPipelineStageFlagBits convertPipelineStage(PipelineType type, ShaderStage shaderStage);

// HU -> VK
VkDescriptorType convertResourceType(ResourceType resource);

// HU -> VK
VkFormat convertDataFormat(GraphicsDataFormat format);

// HU -> VK
VkVertexInputRate convertVertexInputRate(VertexInputRate inputRate);

// HU -> VK
VkBufferUsageFlagBits convertBufferUsage(BufferUsageFlags usage);

// VK -> HU
GraphicsDataFormat convertVkFormat(VkFormat format);

} // namespace huedra::converter