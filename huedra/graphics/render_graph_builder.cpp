#include "render_graph_builder.hpp"
#include "core/log.hpp"

namespace huedra {

RenderGraphBuilder& RenderGraphBuilder::init()
{
    m_renderPasses.clear();
    return *this;
}

RenderGraphBuilder& RenderGraphBuilder::addGraphicsPass(const std::string& name, const PipelineBuilder& pipeline,
                                                        Ref<RenderTarget> renderTarget, RenderCommands renderCommands,
                                                        const std::vector<std::string>& dependencies,
                                                        RenderPassSettings settings)
{
    if (pipeline.getType() != PipelineType::GRAPHICS)
    {
        log(LogLevel::WARNING, "Could not add render pass, pipeline not a graphics pipeline");
        return *this;
    }

    if (!renderTarget.valid())
    {
        log(LogLevel::WARNING, "Could not add render pass, renderTarget not valid");
        return *this;
    }

    if (!renderCommands)
    {
        log(LogLevel::WARNING, "Could not add render pass, no renderCommands present");
        return *this;
    }

    if (m_renderPasses.contains(name))
    {
        log(LogLevel::WARNING, "Could not add render pass, render pass: %s already exists", name.c_str());
        return *this;
    }

    for (auto& dependency : dependencies)
    {
        if (m_renderPasses.contains(dependency))
        {
            log(LogLevel::WARNING, "Could not add render pass, render pass dependency: %s not defined",
                dependency.c_str());
            return *this;
        }
    }

    m_renderPasses.insert(
        std::pair<std::string, RenderPassInfo>(name, {pipeline, renderTarget, renderCommands, dependencies, settings}));
    return *this;
}

} // namespace huedra