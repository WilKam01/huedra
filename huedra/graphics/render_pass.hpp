#pragma once

#include "core/references/ref.hpp"
#include "core/types.hpp"
#include "graphics/pipeline.hpp"
#include "graphics/render_target.hpp"

namespace huedra {

class RenderPass
{
public:
    RenderPass() = default;
    virtual ~RenderPass() = default;

    void initGraphics(const std::string name, Ref<Pipeline> pipeline, Ref<RenderTarget> renderTarget);
    void cleanup();

    Ref<Pipeline> getPipeline() { return m_pipeline; }
    Ref<RenderTarget> getRenderTarget() { return m_renderTarget; }

private:
    Ref<Pipeline> m_pipeline{nullptr};
    Ref<RenderTarget> m_renderTarget{nullptr};
};

} // namespace huedra