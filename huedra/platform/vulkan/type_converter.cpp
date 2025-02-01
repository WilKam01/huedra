#include "type_converter.hpp"

namespace huedra::converter {

VkPipelineBindPoint convertPipelineType(PipelineType type)
{
    switch (type)
    {
    case PipelineType::GRAPHICS:
        return VK_PIPELINE_BIND_POINT_GRAPHICS;
    case PipelineType::COMPUTE:
        return VK_PIPELINE_BIND_POINT_COMPUTE;
    };
}

VkShaderStageFlagBits convertShaderStage(PipelineType type, ShaderStageFlags shaderStage)
{
    u32 result = 0;

    switch (type)
    {
    case PipelineType::GRAPHICS:
        if ((shaderStage & HU_SHADER_STAGE_GRAPHICS_ALL) == HU_SHADER_STAGE_GRAPHICS_ALL)
        {
            return VK_SHADER_STAGE_ALL_GRAPHICS;
        }

        if (shaderStage & HU_SHADER_STAGE_VERTEX)
        {
            result |= VK_SHADER_STAGE_VERTEX_BIT;
        }
        if (shaderStage & HU_SHADER_STAGE_FRAGMENT)
        {
            result |= VK_SHADER_STAGE_FRAGMENT_BIT;
        }
        break;
    case PipelineType::COMPUTE:
        if (shaderStage & HU_SHADER_STAGE_COMPUTE)
        {
            return VK_SHADER_STAGE_COMPUTE_BIT;
        }
    };

    return static_cast<VkShaderStageFlagBits>(result);
}

VkPipelineStageFlagBits convertPipelineStage(PipelineType type, ShaderStageFlags shaderStage)
{
    u32 result = 0;

    switch (type)
    {
    case PipelineType::GRAPHICS:
        if ((shaderStage & HU_SHADER_STAGE_GRAPHICS_ALL) == HU_SHADER_STAGE_GRAPHICS_ALL)
        {
            return VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
        }

        if (shaderStage & HU_SHADER_STAGE_VERTEX)
        {
            result |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
        }
        if (shaderStage & HU_SHADER_STAGE_FRAGMENT)
        {
            result |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        break;
    case PipelineType::COMPUTE:
        if (shaderStage & HU_SHADER_STAGE_COMPUTE)
        {
            return VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        }
    };

    return static_cast<VkPipelineStageFlagBits>(result);
}

VkDescriptorType convertResourceType(ResourceType resource)
{
    switch (resource)
    {
    case ResourceType::UNIFORM_BUFFER:
        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    case ResourceType::STORAGE_BUFFER:
        return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    case ResourceType::UNFIFORM_TEXTURE:
        return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    case ResourceType::STORAGE_TEXTURE:
        return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

    default:
        return VK_DESCRIPTOR_TYPE_MAX_ENUM;
    };
}

VkFormat convertDataFormat(GraphicsDataFormat format)
{
    switch (format)
    {
    case GraphicsDataFormat::UNDEFINED:
        return VK_FORMAT_UNDEFINED;

    case GraphicsDataFormat::R_8_INT:
        return VK_FORMAT_R8_SINT;
    case GraphicsDataFormat::R_8_UINT:
        return VK_FORMAT_R8_UINT;
    case GraphicsDataFormat::R_8_NORM:
        return VK_FORMAT_R8_SNORM;
    case GraphicsDataFormat::R_8_UNORM:
        return VK_FORMAT_R8_UNORM;
    case GraphicsDataFormat::R_16_INT:
        return VK_FORMAT_R16_SINT;
    case GraphicsDataFormat::R_16_UINT:
        return VK_FORMAT_R16_UINT;
    case GraphicsDataFormat::R_16_FLOAT:
        return VK_FORMAT_R16_SFLOAT;
    case GraphicsDataFormat::R_16_NORM:
        return VK_FORMAT_R16_SNORM;
    case GraphicsDataFormat::R_16_UNORM:
        return VK_FORMAT_R16_UNORM;
    case GraphicsDataFormat::R_32_INT:
        return VK_FORMAT_R32_SINT;
    case GraphicsDataFormat::R_32_UINT:
        return VK_FORMAT_R32_UINT;
    case GraphicsDataFormat::R_32_FLOAT:
        return VK_FORMAT_R32_SFLOAT;
    case GraphicsDataFormat::R_64_INT:
        return VK_FORMAT_R64_SINT;
    case GraphicsDataFormat::R_64_UINT:
        return VK_FORMAT_R64_UINT;
    case GraphicsDataFormat::R_64_FLOAT:
        return VK_FORMAT_R64_SFLOAT;

    case GraphicsDataFormat::RG_8_INT:
        return VK_FORMAT_R8G8_SINT;
    case GraphicsDataFormat::RG_8_UINT:
        return VK_FORMAT_R8G8_UINT;
    case GraphicsDataFormat::RG_8_NORM:
        return VK_FORMAT_R8G8_SNORM;
    case GraphicsDataFormat::RG_8_UNORM:
        return VK_FORMAT_R8G8_UNORM;
    case GraphicsDataFormat::RG_16_INT:
        return VK_FORMAT_R16G16_SINT;
    case GraphicsDataFormat::RG_16_UINT:
        return VK_FORMAT_R16G16_UINT;
    case GraphicsDataFormat::RG_16_FLOAT:
        return VK_FORMAT_R16G16_SFLOAT;
    case GraphicsDataFormat::RG_16_NORM:
        return VK_FORMAT_R16G16_SNORM;
    case GraphicsDataFormat::RG_16_UNORM:
        return VK_FORMAT_R16G16_UNORM;
    case GraphicsDataFormat::RG_32_INT:
        return VK_FORMAT_R32G32_SINT;
    case GraphicsDataFormat::RG_32_UINT:
        return VK_FORMAT_R32G32_UINT;
    case GraphicsDataFormat::RG_32_FLOAT:
        return VK_FORMAT_R32G32_SFLOAT;
    case GraphicsDataFormat::RG_64_INT:
        return VK_FORMAT_R64G64_SINT;
    case GraphicsDataFormat::RG_64_UINT:
        return VK_FORMAT_R64G64_UINT;
    case GraphicsDataFormat::RG_64_FLOAT:
        return VK_FORMAT_R64G64_SFLOAT;

    case GraphicsDataFormat::RGB_8_INT:
        return VK_FORMAT_R8G8B8_SINT;
    case GraphicsDataFormat::RGB_8_UINT:
        return VK_FORMAT_R8G8B8_UINT;
    case GraphicsDataFormat::RGB_8_NORM:
        return VK_FORMAT_R8G8B8_SNORM;
    case GraphicsDataFormat::RGB_8_UNORM:
        return VK_FORMAT_R8G8B8_UNORM;
    case GraphicsDataFormat::RGB_16_INT:
        return VK_FORMAT_R16G16B16_SINT;
    case GraphicsDataFormat::RGB_16_UINT:
        return VK_FORMAT_R16G16B16_UINT;
    case GraphicsDataFormat::RGB_16_FLOAT:
        return VK_FORMAT_R16G16B16_SFLOAT;
    case GraphicsDataFormat::RGB_16_NORM:
        return VK_FORMAT_R16G16B16_SNORM;
    case GraphicsDataFormat::RGB_16_UNORM:
        return VK_FORMAT_R16G16B16_UNORM;
    case GraphicsDataFormat::RGB_32_INT:
        return VK_FORMAT_R32G32B32_SINT;
    case GraphicsDataFormat::RGB_32_UINT:
        return VK_FORMAT_R32G32B32_UINT;
    case GraphicsDataFormat::RGB_32_FLOAT:
        return VK_FORMAT_R32G32B32_SFLOAT;
    case GraphicsDataFormat::RGB_64_INT:
        return VK_FORMAT_R64G64B64_SINT;
    case GraphicsDataFormat::RGB_64_UINT:
        return VK_FORMAT_R64G64B64_UINT;
    case GraphicsDataFormat::RGB_64_FLOAT:
        return VK_FORMAT_R64G64B64_SFLOAT;

    case GraphicsDataFormat::RGBA_8_INT:
        return VK_FORMAT_R8G8B8A8_SINT;
    case GraphicsDataFormat::RGBA_8_UINT:
        return VK_FORMAT_R8G8B8A8_UINT;
    case GraphicsDataFormat::RGBA_8_NORM:
        return VK_FORMAT_R8G8B8A8_SNORM;
    case GraphicsDataFormat::RGBA_8_UNORM:
        return VK_FORMAT_R8G8B8A8_UNORM;
    case GraphicsDataFormat::RGBA_16_INT:
        return VK_FORMAT_R16G16B16A16_SINT;
    case GraphicsDataFormat::RGBA_16_UINT:
        return VK_FORMAT_R16G16B16A16_UINT;
    case GraphicsDataFormat::RGBA_16_FLOAT:
        return VK_FORMAT_R16G16B16A16_SFLOAT;
    case GraphicsDataFormat::RGBA_16_NORM:
        return VK_FORMAT_R16G16B16A16_SNORM;
    case GraphicsDataFormat::RGBA_16_UNORM:
        return VK_FORMAT_R16G16B16A16_UNORM;
    case GraphicsDataFormat::RGBA_32_INT:
        return VK_FORMAT_R32G32B32A32_SINT;
    case GraphicsDataFormat::RGBA_32_UINT:
        return VK_FORMAT_R32G32B32A32_UINT;
    case GraphicsDataFormat::RGBA_32_FLOAT:
        return VK_FORMAT_R32G32B32A32_SFLOAT;
    case GraphicsDataFormat::RGBA_64_INT:
        return VK_FORMAT_R64G64B64A64_SINT;
    case GraphicsDataFormat::RGBA_64_UINT:
        return VK_FORMAT_R64G64B64A64_UINT;
    case GraphicsDataFormat::RGBA_64_FLOAT:
        return VK_FORMAT_R64G64B64A64_SFLOAT;
    }
}

VkVertexInputRate convertVertexInputRate(VertexInputRate inputRate)
{
    switch (inputRate)
    {
    case VertexInputRate::VERTEX:
        return VK_VERTEX_INPUT_RATE_VERTEX;
    case VertexInputRate::INSTANCE:
        return VK_VERTEX_INPUT_RATE_INSTANCE;
    }
}

VkBufferUsageFlagBits convertBufferUsage(BufferUsageFlags usage)
{
    u32 result = 0;

    if (usage & HU_BUFFER_USAGE_VERTEX_BUFFER)
    {
        result |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }

    if (usage & HU_BUFFER_USAGE_INDEX_BUFFER)
    {
        result |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }

    if (usage & HU_BUFFER_USAGE_UNIFORM_BUFFER)
    {
        result |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }

    if (usage & HU_BUFFER_USAGE_STORAGE_BUFFER)
    {
        result |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }

    return static_cast<VkBufferUsageFlagBits>(result);
}

GraphicsDataFormat convertVkFormat(VkFormat format)
{
    switch (format)
    {
    case VK_FORMAT_UNDEFINED:
        return GraphicsDataFormat::UNDEFINED;

    case VK_FORMAT_R8_SINT:
        return GraphicsDataFormat::R_8_INT;
    case VK_FORMAT_R8_UINT:
        return GraphicsDataFormat::R_8_UINT;
    case VK_FORMAT_R8_SNORM:
        return GraphicsDataFormat::R_8_NORM;
    case VK_FORMAT_R8_UNORM:
        return GraphicsDataFormat::R_8_UNORM;
    case VK_FORMAT_R16_SINT:
        return GraphicsDataFormat::R_16_INT;
    case VK_FORMAT_R16_UINT:
        return GraphicsDataFormat::R_16_UINT;
    case VK_FORMAT_R16_SFLOAT:
        return GraphicsDataFormat::R_16_FLOAT;
    case VK_FORMAT_R16_SNORM:
        return GraphicsDataFormat::R_16_NORM;
    case VK_FORMAT_R16_UNORM:
        return GraphicsDataFormat::R_16_UNORM;
    case VK_FORMAT_R32_SINT:
        return GraphicsDataFormat::R_32_INT;
    case VK_FORMAT_R32_UINT:
        return GraphicsDataFormat::R_32_UINT;
    case VK_FORMAT_R32_SFLOAT:
        return GraphicsDataFormat::R_32_FLOAT;
    case VK_FORMAT_R64_SINT:
        return GraphicsDataFormat::R_64_INT;
    case VK_FORMAT_R64_UINT:
        return GraphicsDataFormat::R_64_UINT;
    case VK_FORMAT_R64_SFLOAT:
        return GraphicsDataFormat::R_64_FLOAT;

    case VK_FORMAT_R8G8_SINT:
        return GraphicsDataFormat::RG_8_INT;
    case VK_FORMAT_R8G8_UINT:
        return GraphicsDataFormat::RG_8_UINT;
    case VK_FORMAT_R8G8_SNORM:
        return GraphicsDataFormat::RG_8_NORM;
    case VK_FORMAT_R8G8_UNORM:
        return GraphicsDataFormat::RG_8_UNORM;
    case VK_FORMAT_R16G16_SINT:
        return GraphicsDataFormat::RG_16_INT;
    case VK_FORMAT_R16G16_UINT:
        return GraphicsDataFormat::RG_16_UINT;
    case VK_FORMAT_R16G16_SFLOAT:
        return GraphicsDataFormat::RG_16_FLOAT;
    case VK_FORMAT_R16G16_SNORM:
        return GraphicsDataFormat::RG_16_NORM;
    case VK_FORMAT_R16G16_UNORM:
        return GraphicsDataFormat::RG_16_UNORM;
    case VK_FORMAT_R32G32_SINT:
        return GraphicsDataFormat::RG_32_INT;
    case VK_FORMAT_R32G32_UINT:
        return GraphicsDataFormat::RG_32_UINT;
    case VK_FORMAT_R32G32_SFLOAT:
        return GraphicsDataFormat::RG_32_FLOAT;
    case VK_FORMAT_R64G64_SINT:
        return GraphicsDataFormat::RG_64_INT;
    case VK_FORMAT_R64G64_UINT:
        return GraphicsDataFormat::RG_64_UINT;
    case VK_FORMAT_R64G64_SFLOAT:
        return GraphicsDataFormat::RG_64_FLOAT;

    case VK_FORMAT_R8G8B8_SINT:
        return GraphicsDataFormat::RGB_8_INT;
    case VK_FORMAT_R8G8B8_UINT:
        return GraphicsDataFormat::RGB_8_UINT;
    case VK_FORMAT_R8G8B8_SNORM:
        return GraphicsDataFormat::RGB_8_NORM;
    case VK_FORMAT_R8G8B8_UNORM:
        return GraphicsDataFormat::RGB_8_UNORM;
    case VK_FORMAT_R16G16B16_SINT:
        return GraphicsDataFormat::RGB_16_INT;
    case VK_FORMAT_R16G16B16_UINT:
        return GraphicsDataFormat::RGB_16_UINT;
    case VK_FORMAT_R16G16B16_SFLOAT:
        return GraphicsDataFormat::RGB_16_FLOAT;
    case VK_FORMAT_R16G16B16_SNORM:
        return GraphicsDataFormat::RGB_16_NORM;
    case VK_FORMAT_R16G16B16_UNORM:
        return GraphicsDataFormat::RGB_16_UNORM;
    case VK_FORMAT_R32G32B32_SINT:
        return GraphicsDataFormat::RGB_32_INT;
    case VK_FORMAT_R32G32B32_UINT:
        return GraphicsDataFormat::RGB_32_UINT;
    case VK_FORMAT_R32G32B32_SFLOAT:
        return GraphicsDataFormat::RGB_32_FLOAT;
    case VK_FORMAT_R64G64B64_SINT:
        return GraphicsDataFormat::RGB_64_INT;
    case VK_FORMAT_R64G64B64_UINT:
        return GraphicsDataFormat::RGB_64_UINT;
    case VK_FORMAT_R64G64B64_SFLOAT:
        return GraphicsDataFormat::RGB_64_FLOAT;

    case VK_FORMAT_R8G8B8A8_SINT:
        return GraphicsDataFormat::RGBA_8_INT;
    case VK_FORMAT_R8G8B8A8_UINT:
        return GraphicsDataFormat::RGBA_8_UINT;
    case VK_FORMAT_R8G8B8A8_SNORM:
        return GraphicsDataFormat::RGBA_8_NORM;
    case VK_FORMAT_R8G8B8A8_UNORM:
        return GraphicsDataFormat::RGBA_8_UNORM;
    case VK_FORMAT_R16G16B16A16_SINT:
        return GraphicsDataFormat::RGBA_16_INT;
    case VK_FORMAT_R16G16B16A16_UINT:
        return GraphicsDataFormat::RGBA_16_UINT;
    case VK_FORMAT_R16G16B16A16_SFLOAT:
        return GraphicsDataFormat::RGBA_16_FLOAT;
    case VK_FORMAT_R16G16B16A16_SNORM:
        return GraphicsDataFormat::RGBA_16_NORM;
    case VK_FORMAT_R16G16B16A16_UNORM:
        return GraphicsDataFormat::RGBA_16_UNORM;
    case VK_FORMAT_R32G32B32A32_SINT:
        return GraphicsDataFormat::RGBA_32_INT;
    case VK_FORMAT_R32G32B32A32_UINT:
        return GraphicsDataFormat::RGBA_32_UINT;
    case VK_FORMAT_R32G32B32A32_SFLOAT:
        return GraphicsDataFormat::RGBA_32_FLOAT;
    case VK_FORMAT_R64G64B64A64_SINT:
        return GraphicsDataFormat::RGBA_64_INT;
    case VK_FORMAT_R64G64B64A64_UINT:
        return GraphicsDataFormat::RGBA_64_UINT;
    case VK_FORMAT_R64G64B64A64_SFLOAT:
        return GraphicsDataFormat::RGBA_64_FLOAT;

    default:
        return GraphicsDataFormat::UNDEFINED;
    }
}

} // namespace huedra::converter
