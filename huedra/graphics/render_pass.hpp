#pragma once

#include "core/references/ref.hpp"
#include "core/types.hpp"
#include "graphics/pipeline.hpp"
#include "graphics/render_target.hpp"

#include <functional>

namespace huedra {

class RenderPass
{
public:
    RenderPass() = default;
    virtual ~RenderPass() = default;

    void initGraphics(const std::string name, Ref<Pipeline> pipeline, Ref<RenderTarget> renderTarget,
                      std::function<void(RenderContext&)> renderCommands);
    void cleanup();

    Ref<Pipeline> getPipeline() { return m_pipeline; }
    Ref<RenderTarget> getRenderTarget() { return m_renderTarget; }
    std::function<void(RenderContext&)> getRenderCommands() { return m_renderCommands; }

private:
    Ref<Pipeline> m_pipeline{nullptr};
    Ref<RenderTarget> m_renderTarget{nullptr};
    std::function<void(RenderContext&)> m_renderCommands{nullptr};
};

} // namespace huedra