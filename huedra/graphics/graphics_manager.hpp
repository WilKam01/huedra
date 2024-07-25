#pragma

#include "core/references/ref.hpp"
#include "graphics/context.hpp"
#include "graphics/render_pass.hpp"
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

    Ref<Pipeline> createPipeline(const PipelineBuilder& pipelineBuilder);
    void addRenderPass(RenderPass renderPass);

private:
    void createSwapchain(Window* window);
    void removeSwapchain(size_t index);

    GraphicalContext* m_context;
    std::vector<RenderPass> m_renderPasses;
};

} // namespace huedra
