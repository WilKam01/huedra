#pragma once

#include "core/references/ref.hpp"
#include "core/types.hpp"
#include "graphics/pipeline_builder.hpp"
#include "graphics/render_target.hpp"
#include "math/vec3.hpp"
#include "render_pass_builder.hpp"

#include <functional>

namespace huedra {

using RenderCommands = std::function<void(RenderContext&)>;

struct RenderPassSettings
{
    std::array<float, 3> clearColor{{0.0f, 0.0f, 0.0f}};
    bool clearRenderTarget{true};
};

struct RenderPassInfo
{
    PipelineBuilder pipeline;
    Ref<RenderTarget> renderTarget;
    RenderCommands commands;
    std::vector<std::string> dependencies;
    RenderPassSettings settings;
};

class RenderGraphBuilder
{
public:
    RenderGraphBuilder() = default;
    ~RenderGraphBuilder() = default;

    RenderGraphBuilder& init();
    RenderGraphBuilder& addPass(const std::string& name, const RenderPassBuilder& pass);

    u64 generateHash();

    u64 getHash() const { return m_hash; }
    std::map<std::string, RenderPassBuilder> getRenderPasses() const { return m_passes; }

private:
    u64 m_hash{0};

    std::map<std::string, RenderPassBuilder> m_passes;
};

} // namespace huedra