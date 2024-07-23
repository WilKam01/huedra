#include "render_pass.hpp"

namespace huedra {

void RenderPass::initGraphics(const std::string name, Ref<RenderTarget> renderTarget) { m_renderTarget = renderTarget; }

} // namespace huedra