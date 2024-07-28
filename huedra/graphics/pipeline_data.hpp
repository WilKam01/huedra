#pragma once

#include "core/types.hpp"

namespace huedra {

enum class PipelineType
{
    GRAPHICS
};

enum class ShaderStage
{
    VERTEX,
    FRAGMENT
};

typedef enum ShaderStageFlags
{
    HU_SHADER_STAGE_NONE = 0x0000,

    HU_SHADER_STAGE_VERTEX = 0x0001,
    HU_SHADER_STAGE_FRAGMENT = 0x0002,
    HU_SHADER_STAGE_GRAPHICS_ALL = 0x0003,

    HU_SHADER_STAGE_ALL = 0xFFFF
} ShaderStageFlags;

enum class ResourceType
{
    UNIFORM_BUFFER,
    STRUCTURED_BUFFER,
    TEXTURE,
};

enum class BufferType
{
    STATIC,
    DYNAMIC
};

enum BufferUsageFlags
{
    HU_BUFFER_USAGE_UNDEFINED = 0,
    HU_BUFFER_USAGE_VERTEX_BUFFER,
    HU_BUFFER_USAGE_INDEX_BUFFER,
    HU_BUFFER_USAGE_UNIFORM_BUFFER,
    HU_BUFFER_USAGE_STORAGE_BUFFER,
};

struct ResourceBinding
{
    ShaderStageFlags shaderStage;
    ResourceType resource;
};

enum class VertexInputRate
{
    VERTEX,
    INSTANCE
};

enum class GraphicsChannels
{
    R,
    RG,
    RGB,
    RGBA
};

enum class BitSize
{
    B8,
    B16,
    B32,
    B64,
};

enum class GraphicsDataType
{
    INT,
    UINT,
    FLOAT
};

enum class GraphicsDataFormat
{
    R_8_INT,
    R_8_UINT,
    R_8_NORM,
    R_8_UNORM,
    R_16_INT,
    R_16_UINT,
    R_16_FLOAT,
    R_16_NORM,
    R_16_UNORM,
    R_32_INT,
    R_32_UINT,
    R_32_FLOAT,
    R_64_INT,
    R_64_UINT,
    R_64_FLOAT,

    RG_8_INT,
    RG_8_UINT,
    RG_8_NORM,
    RG_8_UNORM,
    RG_16_INT,
    RG_16_UINT,
    RG_16_FLOAT,
    RG_16_NORM,
    RG_16_UNORM,
    RG_32_INT,
    RG_32_UINT,
    RG_32_FLOAT,
    RG_64_INT,
    RG_64_UINT,
    RG_64_FLOAT,

    RGB_8_INT,
    RGB_8_UINT,
    RGB_8_NORM,
    RGB_8_UNORM,
    RGB_16_INT,
    RGB_16_UINT,
    RGB_16_FLOAT,
    RGB_16_NORM,
    RGB_16_UNORM,
    RGB_32_INT,
    RGB_32_UINT,
    RGB_32_FLOAT,
    RGB_64_INT,
    RGB_64_UINT,
    RGB_64_FLOAT,

    RGBA_8_INT,
    RGBA_8_UINT,
    RGBA_8_NORM,
    RGBA_8_UNORM,
    RGBA_16_INT,
    RGBA_16_UINT,
    RGBA_16_FLOAT,
    RGBA_16_NORM,
    RGBA_16_UNORM,
    RGBA_32_INT,
    RGBA_32_UINT,
    RGBA_32_FLOAT,
    RGBA_64_INT,
    RGBA_64_UINT,
    RGBA_64_FLOAT,
};

struct VertexInputAttribute
{
    GraphicsDataFormat format;
    u32 offset;
};

struct VertexInputStream
{
    u32 size;
    VertexInputRate inputRate;
    std::vector<VertexInputAttribute> attributes;
};

} // namespace huedra
