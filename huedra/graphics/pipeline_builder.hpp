#pragma once

#include "core/types.hpp"

#include <stdio.h>

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
    SHADER_STAGE_NONE = 0x0000,

    SHADER_STAGE_VERTEX = 0x0001,
    SHADER_STAGE_FRAGMENT = 0x0002,
    SHADER_STAGE_GRAPHICS_ALL = 0x0003,

    SHADER_STAGE_ALL = 0xFFFF
} ShaderStageFlags;

enum class ResourceType
{
    UNIFORM_BUFFER,
    TEXTURE,
};

struct ResourceBinding
{
    ShaderStageFlags shaderStage;
    ResourceType resource;
};

class PipelineBuilder
{
public:
    PipelineBuilder() = default;
    ~PipelineBuilder() = default;

    PipelineBuilder& init(PipelineType type);

    PipelineBuilder& addShader(ShaderStage stage, std::string shader);

    PipelineBuilder& addPushConstantRange(u32 stage, u32 size);
    PipelineBuilder& addResourceSet();
    PipelineBuilder& addResourceBinding(u32 stage, ResourceType resource);

    PipelineType getType() const { return m_type; }
    std::map<ShaderStage, std::string> getShaderStages() const { return m_shaderStages; }
    std::vector<std::vector<ResourceBinding>> getResources() const { return m_resources; }

    u32 getPushConstantRange() { return m_pushConstantRange; }
    ShaderStageFlags getPushConstantShaderStage() { return m_pushConstantShaderStage; }

private:
    PipelineType m_type;
    std::map<ShaderStage, std::string> m_shaderStages;
    std::vector<std::vector<ResourceBinding>> m_resources{};

    u32 m_pushConstantRange{0};
    ShaderStageFlags m_pushConstantShaderStage{SHADER_STAGE_NONE};
};

} // namespace huedra