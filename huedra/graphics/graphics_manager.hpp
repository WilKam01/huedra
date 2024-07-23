#pragma

#include "core/references/ref.hpp"
#include "graphics/context.hpp"
#include "graphics/render_target.hpp"
#include "window/window.hpp"

namespace huedra {

class GraphicsManager
{
    friend class WindowManager;

public:
    static constexpr u32 MAX_FRAMES_IN_FLIGHT = 2;

    GraphicsManager() = default;
    ~GraphicsManager() = default;

    void init();
    void cleanup();
    void render();

    void addRenderTarget(Ref<RenderTarget> renderTarget);

private:
    void createSwapchain(Window* window);
    void removeSwapchain(size_t index);

    GraphicalContext* m_context;
    std::vector<Ref<RenderTarget>> m_renderTargets;
};

} // namespace huedra
