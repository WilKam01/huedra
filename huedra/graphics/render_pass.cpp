#include "render_pass.hpp"

namespace huedra {

void RenderPass::initGraphics(const std::string name, Ref<Pipeline> pipeline, Ref<RenderTarget> renderTarget)
{
    m_pipeline = pipeline;
    m_renderTarget = renderTarget;
}

void RenderPass::cleanup() {}

} // namespace huedra