#pragma once

#include "core/types.hpp"

namespace huedra {

enum class PipelineType
{
    GRAPHICS,
    COMPUTE
};

enum class ShaderStage
{
    NONE,
    VERTEX,
    FRAGMENT,
    COMPUTE,
    ALL,
};

// TODO: Create this automatically? Macro?
constexpr std::array<std::string_view, 5> ShaderStageNames{"None", "Vertex", "Fragment", "Compute", "All"};

enum class ResourceType
{
    NONE,              // Invalid or not supported
    CONSTANT_BUFFER,   // Read only
    STRUCTURED_BUFFER, // Read/Write
    TEXTURE,           // Read only
    RW_TEXTURE,        // Read/Write
    SAMPLER,
};

// TODO: Create this automatically? Macro?
constexpr std::array<std::string_view, 6> ResourceTypeNames{"None",    "Constant Buffer",    "Structured Buffer",
                                                            "Texture", "Read/Write Texture", "Sampler"};

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
    HU_BUFFER_USAGE_CONSTANT_BUFFER,
    HU_BUFFER_USAGE_STRUCTURED_BUFFER,
};

struct ResourceBinding
{
    std::string name;
    ShaderStage shaderStage;
    ResourceType type;
    bool partOfArgBuffer{false}; // Only for Metal
};

struct ResourcePosition
{
    ResourceBinding info;
    u32 set{0};
    u32 binding{0};
};

struct ParameterBinding
{
    std::string name;
    ShaderStage shaderStage;
    u32 offset;
    u32 size;
};

// TODO: Add all combinations of values, make it less Vulkan dependant, add checks for availability based on API,
// platform etc.
enum class GraphicsDataFormat
{
    UNDEFINED,

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
    BGR_8_INT,
    BGR_8_UINT,
    BGR_8_NORM,
    BGR_8_UNORM,
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
    BGRA_8_INT,
    BGRA_8_UINT,
    BGRA_8_NORM,
    BGRA_8_UNORM,
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

enum class VertexInputRate
{
    VERTEX,
    INSTANCE
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

// Primitive to render
enum class PrimitiveType
{
    POINT,
    LINE,
    TRIANGLE
};

// Vertex data structure
enum class PrimitiveLayout
{
    POINT_LIST,
    LINE_LIST,
    LINE_STRIP,
    TRIANGLE_LIST,
    TRIANGLE_STRIP
};

enum class SamplerFilter
{
    NEAREST,
    LINEAR
};

enum class SamplerAddressMode
{
    REPEAT,
    MIRROR_REPEAT,
    CLAMP_EDGE,
    CLAMP_COLOR,
};

enum class SamplerColor
{
    WHITE,
    BLACK,
    ZERO_ALPHA
};

struct SamplerSettings
{
    SamplerFilter filter{SamplerFilter::NEAREST};
    SamplerAddressMode adressModeU{SamplerAddressMode::REPEAT};
    SamplerAddressMode adressModeV{SamplerAddressMode::REPEAT};
    SamplerAddressMode adressModeW{SamplerAddressMode::REPEAT};
    SamplerColor color{SamplerColor::WHITE}; // Only relevant when address mode CLAMP_COLOR is used

    bool operator==(const SamplerSettings& rhs) const
    {
        return filter == rhs.filter && adressModeU == rhs.adressModeU && adressModeV == rhs.adressModeV &&
               adressModeW == rhs.adressModeW && color == rhs.color;
    }
};

constexpr SamplerSettings SAMPLER_NEAR{.filter = SamplerFilter::NEAREST,
                                       .adressModeU = SamplerAddressMode::REPEAT,
                                       .adressModeV = SamplerAddressMode::REPEAT,
                                       .adressModeW = SamplerAddressMode::REPEAT,
                                       .color = SamplerColor::WHITE};

constexpr SamplerSettings SAMPLER_LINEAR{.filter = SamplerFilter::LINEAR,
                                         .adressModeU = SamplerAddressMode::REPEAT,
                                         .adressModeV = SamplerAddressMode::REPEAT,
                                         .adressModeW = SamplerAddressMode::REPEAT,
                                         .color = SamplerColor::WHITE};

} // namespace huedra
