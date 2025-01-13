#pragma once

#include "core/references/ref.hpp"
#include "core/types.hpp"
#include "graphics/pipeline_builder.hpp"
#include "graphics/render_target.hpp"
#include "math/vec3.hpp"
#include "render_pass_builder.hpp"

#include <functional>

namespace huedra {

using RenderCommands = std::function<void(RenderContext&)>;

struct RenderPassSettings
{
    std::array<float, 3> clearColor{{0.0f, 0.0f, 0.0f}};
    bool clearRenderTarget{true};
};

struct RenderPassInfo
{
    PipelineBuilder pipeline;
    Ref<RenderTarget> renderTarget;
    RenderCommands commands;
    std::vector<std::string> dependencies;
    RenderPassSettings settings;
};

class RenderGraphBuilder
{
public:
    RenderGraphBuilder() = default;
    ~RenderGraphBuilder() = default;

    RenderGraphBuilder& init();
    RenderGraphBuilder& addPass(const std::string& name, const RenderPassBuilder& pass);

    Ref<Buffer> addBufferResource(BufferUsageFlags usage, u64 size);
    Ref<Texture> addTextureResource(u32 width, u32 height, GraphicsDataFormat format);

    u64 generateHash();

    bool empty() const { return m_passes.empty(); }
    u64 getHash() const { return m_hash; }
    std::map<std::string, RenderPassBuilder> getRenderPasses() const { return m_passes; }
    std::vector<std::shared_ptr<Buffer>> getBuffers() const { return m_buffers; }
    std::vector<std::shared_ptr<Texture>> getTextures() const { return m_textures; }

private:
    u64 m_hash{0};

    std::map<std::string, RenderPassBuilder> m_passes;
    std::vector<std::shared_ptr<Buffer>> m_buffers;
    std::vector<std::shared_ptr<Texture>> m_textures;
};

} // namespace huedra