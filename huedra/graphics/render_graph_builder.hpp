#pragma once

#include "core/references/ref.hpp"
#include "core/types.hpp"
#include "graphics/pipeline_builder.hpp"
#include "graphics/render_target.hpp"

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

    RenderGraphBuilder& addGraphicsPass(const std::string& name, const PipelineBuilder& pipeline,
                                        Ref<RenderTarget> renderTarget, RenderCommands renderCommands,
                                        const std::vector<std::string>& dependencies = {},
                                        RenderPassSettings settings = {});

    std::map<std::string, RenderPassInfo> getRenderPasses() const { return m_renderPasses; }

private:
    std::map<std::string, RenderPassInfo> m_renderPasses;
};

} // namespace huedra