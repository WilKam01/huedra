#pragma

#include "core/references/ref.hpp"
#include "graphics/buffer.hpp"
#include "graphics/context.hpp"
#include "graphics/render_graph_builder.hpp"
#include "graphics/resource_set.hpp"
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

    Ref<Buffer> createBuffer(BufferType type, u32 usage, u64 size, void* data = nullptr);
    Ref<ResourceSet> createResourceSet(const std::string renderPass, u32 setIndex);

    void setRenderGraph(RenderGraphBuilder& builder);

    u32 getCurrentFrame() { return m_currentFrame; }

private:
    void createSwapchain(Window* window, bool renderDepth);
    void removeSwapchain(size_t index);

    GraphicalContext* m_context;
    u32 m_currentFrame{0};
};

} // namespace huedra
