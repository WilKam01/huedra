#pragma

#include "core/references/ref.hpp"
#include "graphics/buffer.hpp"
#include "graphics/context.hpp"
#include "graphics/render_graph_builder.hpp"
#include "resources/texture/data.hpp"
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
    void update();
    void render(RenderGraphBuilder& builder);

    Ref<Buffer> createBuffer(BufferType type, u32 usage, u64 size, void* data = nullptr);
    Ref<Texture> createTexture(TextureData textureData);
    Ref<RenderTarget> createRenderTarget(RenderTargetType type, GraphicsDataFormat format, u32 width, u32 height);

    void removeBuffer(Ref<Buffer> buffer);
    void removeTexture(Ref<Texture> texture);
    void removeRenderTarget(Ref<RenderTarget> renderTarget);

    u32 getCurrentFrame() { return m_currentFrame; }

private:
    void createSwapchain(Window* window, bool renderDepth);
    void removeSwapchain(size_t index);

    GraphicalContext* m_context;
    u32 m_currentFrame{0};
};

} // namespace huedra
