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
    bool clearTarget;
};

class RenderPassBuilder
{
public:
    RenderPassBuilder() = default;
    virtual ~RenderPassBuilder() = default;

    RenderPassBuilder& init(RenderPassType type, const PipelineBuilder& pipeline = {});
    RenderPassBuilder& setCommands(const RenderCommands& commands);

    RenderPassBuilder& addResource(ResourceAccessType access, Ref<Buffer> buffer);
    RenderPassBuilder& addResource(ResourceAccessType access, Ref<Texture> texture);
    RenderPassBuilder& addRenderTarget(Ref<RenderTarget> renderTarget, bool clearTarget = true,
                                       vec3 clearColor = vec3(0.0f));

    u64 generateHash();

    bool empty() const { return !m_initialized; }
    RenderPassType getType() const { return m_type; }
    PipelineBuilder getPipeline() const { return m_pipeline; }
    RenderCommands getCommands() const { return m_commands; }
    std::vector<RenderTargetInfo> getRenderTargets() const { return m_renderTargets; }

private:
    bool m_initialized{false};

    RenderPassType m_type;
    PipelineBuilder m_pipeline;
    RenderCommands m_commands{nullptr};

    struct RenderPassReference
    {
        ResourceAccessType access{ResourceAccessType::READ};
        Ref<Buffer> buffer{nullptr};
        Ref<Texture> texture{nullptr};
        enum class ResourceType
        {
            BUFFER,
            TEXTURE,
        } type{ResourceType::BUFFER};
    };

    std::vector<RenderPassReference> m_inputs;
    std::vector<RenderPassReference> m_outputs;

    std::vector<RenderTargetInfo> m_renderTargets;
};

} // namespace huedra