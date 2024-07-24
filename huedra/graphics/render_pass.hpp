#pragma once

#include "core/references/ref.hpp"
#include "core/types.hpp"
#include "graphics/render_target.hpp"

namespace huedra {

class RenderPass
{
public:
    RenderPass() = default;
    virtual ~RenderPass() = default;

    void initGraphics(const std::string name, Ref<RenderTarget> renderTarget);
    void cleanup();

    Ref<RenderTarget> getRenderTarget() { return m_renderTarget; }

private:
    Ref<RenderTarget> m_renderTarget{nullptr};
};

} // namespace huedra