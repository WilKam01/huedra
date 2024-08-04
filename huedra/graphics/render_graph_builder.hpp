#pragma once

#include "core/references/ref.hpp"
#include "core/types.hpp"
#include "graphics/pipeline.hpp"
#include "graphics/render_target.hpp"

#include <functional>

namespace huedra {

using RenderCommands = std::function<void(RenderContext&)>;

struct RenderPassInfo
{
    Ref<Pipeline> pipeline;
    Ref<RenderTarget> renderTarget;
    RenderCommands commands;
    bool clearRenderTarget;
    std::vector<std::string> dependencies;
};

class RenderGraphBuilder
{
public:
    RenderGraphBuilder() = default;
    ~RenderGraphBuilder() = default;

    RenderGraphBuilder& init();

    RenderGraphBuilder& addGraphicsPass(const std::string& name, Ref<Pipeline> pipeline, Ref<RenderTarget> renderTarget,
                                        RenderCommands renderCommands, bool clearRenderTarget = true,
                                        const std::vector<std::string>& dependencies = {});

    std::map<std::string, RenderPassInfo> getRenderPasses() const { return m_renderPasses; }

private:
    std::map<std::string, RenderPassInfo> m_renderPasses;
};

} // namespace huedra