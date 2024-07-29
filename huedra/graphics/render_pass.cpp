#include "render_pass.hpp"

namespace huedra {

void RenderPass::initGraphics(const std::string name, Ref<Pipeline> pipeline, Ref<RenderTarget> renderTarget,
                              std::function<void(RenderContext&)> renderCommands)
{
    m_pipeline = pipeline;
    m_renderTarget = renderTarget;
    m_renderCommands = renderCommands;
}

void RenderPass::cleanup()
{
    m_pipeline = nullptr;
    m_renderTarget = nullptr;
    m_renderCommands = nullptr;
}

} // namespace huedra