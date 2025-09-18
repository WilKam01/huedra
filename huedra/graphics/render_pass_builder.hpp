#pragma once

#include "core/references/ref.hpp"
#include "core/types.hpp"
#include "graphics/pipeline_builder.hpp"
#include "graphics/render_target.hpp"
#include "math/vec3.hpp"

#include <functional>

namespace huedra {

using RenderCommands = std::function<void(RenderContext&)>;

enum class RenderPassType
{
    GRAPHICS,
    COMPUTE,
};

enum class ResourceAccessType
{
    READ,
    WRITE,
    READ_WRITE
};

struct RenderTargetInfo
{
    Ref<RenderTarget> target;
    vec3 clearColor;
};

struct RenderPassReference
{
    ResourceAccessType access{ResourceAccessType::READ};
    ShaderStage shaderStage{ShaderStage::NONE};
    Buffer* buffer{nullptr};
    Texture* texture{nullptr};
    enum class Type
    {
        BUFFER,
        TEXTURE,
    } type{Type::BUFFER};
};

class RenderPassBuilder
{
public:
    RenderPassBuilder() = default;
    virtual ~RenderPassBuilder() = default;

    RenderPassBuilder(const RenderPassBuilder& rhs) = default;
    RenderPassBuilder& operator=(const RenderPassBuilder& rhs) = default;
    RenderPassBuilder(RenderPassBuilder&& rhs) = default;
    RenderPassBuilder& operator=(RenderPassBuilder&& rhs) = default;

    RenderPassBuilder& init(RenderPassType type, RenderTargetType renderTargetUse = RenderTargetType::COLOR_AND_DEPTH);
    RenderPassBuilder& setPipeline(const PipelineBuilder& pipeline);
    RenderPassBuilder& setCommands(const RenderCommands& commands);

    // renderTargetUse only applicable when clearRenderTargets is true
    RenderPassBuilder& setClearRenderTargets(bool clearRenderTargets,
                                             RenderTargetType renderTargetUse = RenderTargetType::COLOR_AND_DEPTH);

    RenderPassBuilder& addResource(ResourceAccessType access, Ref<Buffer> buffer, ShaderStage shaderStage);
    RenderPassBuilder& addResource(ResourceAccessType access, Ref<Texture> texture, ShaderStage shaderStage);
    RenderPassBuilder& addRenderTarget(Ref<RenderTarget> renderTarget, vec3 clearColor = vec3(0.0f));

    u64 generateHash();

    bool empty() const { return m_pipeline.empty() && m_commands == nullptr; }
    RenderPassType getType() const { return m_type; }
    PipelineBuilder getPipeline() const { return m_pipeline; }
    RenderCommands getCommands() const { return m_commands; }
    bool getClearRenderTargets() const { return m_clearTargets; }
    RenderTargetType getRenderTargetUse() const { return m_renderTargetUse; }
    RenderTargetType getRenderTargetClearUse() const { return m_renderTargetClearUse; }
    std::vector<RenderPassReference> getInputs() const { return m_inputs; }
    std::vector<RenderPassReference> getOutputs() const { return m_outputs; }
    std::vector<RenderTargetInfo> getRenderTargets() const { return m_renderTargets; }

private:
    RenderPassType m_type{RenderPassType::GRAPHICS};
    PipelineBuilder m_pipeline;
    RenderCommands m_commands{nullptr};
    bool m_clearTargets{false};
    RenderTargetType m_renderTargetUse{RenderTargetType::COLOR_AND_DEPTH};
    RenderTargetType m_renderTargetClearUse{RenderTargetType::COLOR_AND_DEPTH};

    std::vector<RenderPassReference> m_inputs;
    std::vector<RenderPassReference> m_outputs;

    std::vector<RenderTargetInfo> m_renderTargets;
};

} // namespace huedra