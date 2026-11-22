#pragma

#include "huedra/core/references/ref.hpp"
#include "huedra/graphics/buffer.hpp"
#include "huedra/graphics/context.hpp"
#include "huedra/graphics/render_graph_builder.hpp"
#include "huedra/platform/slang/context.hpp"
#include "huedra/resources/texture/data.hpp"
#include "huedra/window/window.hpp"

namespace huedra {

class GraphicsManager
{
    friend class WindowManager;

public:
    static constexpr u32 MAX_FRAMES_IN_FLIGHT = 2;

    GraphicsManager() = default;
    ~GraphicsManager() = default;

    GraphicsManager(const GraphicsManager& rhs) = delete;
    GraphicsManager& operator=(const GraphicsManager& rhs) = delete;
    GraphicsManager(GraphicsManager&& rhs) = delete;
    GraphicsManager& operator=(GraphicsManager&& rhs) = delete;

    void init();
    void cleanup();
    void update();
    void render(RenderGraphBuilder& builder);

    Ref<Buffer> createBuffer(BufferType type, u32 usage, u64 size, void* data = nullptr);
    Ref<Texture> createTexture(const TextureData& textureData);
    Ref<Texture> createTexture(const HdrTextureData& textureData);
    Ref<RenderTarget> createRenderTarget(RenderTargetType type, GraphicsDataFormat format, u32 width, u32 height);
    ShaderModule createShaderModule(std::string_view name, const u8* sourceCode, u64 sourceCodeLength);
    ShaderModule createShaderModule(std::string_view name, std::string& sourceString);
    CompiledShaderModule compileAndLinkShaderModules(const std::map<ShaderStage, ShaderInput>& inputs);

    void removeBuffer(Ref<Buffer> buffer);
    void removeTexture(Ref<Texture> texture);
    void removeRenderTarget(Ref<RenderTarget> renderTarget);

    u32 getCurrentFrame() const { return m_currentFrame; }

private:
    void createSwapchain(Window* window, bool renderDepth);
    void removeSwapchain(u64 index);

    bool isInitialized() const { return m_context != nullptr; }

    GraphicalContext* m_context{nullptr};
    SlangContext m_slangContext;
    u32 m_currentFrame{0};
};

} // namespace huedra
