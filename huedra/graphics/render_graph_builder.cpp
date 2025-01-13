#include "render_graph_builder.hpp"
#include "core/log.hpp"
#include "graphics/buffer.hpp"
#include "graphics/texture.hpp"

namespace huedra {

RenderGraphBuilder& RenderGraphBuilder::init()
{
    m_passes.clear();
    m_buffers.clear();
    m_textures.clear();
    return *this;
}

RenderGraphBuilder& RenderGraphBuilder::addPass(const std::string& name, const RenderPassBuilder& pass)
{
    if (m_passes.contains(name))
    {
        log(LogLevel::WARNING, "Could not add render pass, %s already exists", name.c_str());
        return *this;
    }

    if (pass.empty())
    {
        log(LogLevel::WARNING, "Could not add render pass, is empty");
        return *this;
    }

    if (!pass.getCommands())
    {
        log(LogLevel::WARNING, "Could not add render pass, no commands set");
        return *this;
    }

    if (pass.getType() == RenderPassType::GRAPHICS && pass.getRenderTargets().empty())
    {
        log(LogLevel::WARNING, "Could not add graphics render pass without any render targets");
        return *this;
    }

    m_passes.insert(std::pair<std::string, RenderPassBuilder>(name, pass));
    return *this;
}

Ref<Buffer> RenderGraphBuilder::addBufferResource(BufferUsageFlags usage, u64 size)
{
    Buffer* buffer = new Buffer();
    buffer->init(BufferType::STATIC, usage, size, nullptr);
    m_buffers.push_back(std::shared_ptr<Buffer>(buffer));
    return Ref<Buffer>(buffer);
}

Ref<Texture> RenderGraphBuilder::addTextureResource(u32 width, u32 height, GraphicsDataFormat format)
{
    Texture* texture = new Texture();
    texture->init(width, height, format, TextureType::COLOR, nullptr);
    m_textures.push_back(std::shared_ptr<Texture>(texture));
    return Ref<Texture>(texture);
}

u64 RenderGraphBuilder::generateHash()
{
    u64 fnvPrime = 0x00000100000001b3;
    m_hash = 0xcbf29ce484222325;
    auto combineHash = [this, fnvPrime](u64 val) { m_hash ^= val * fnvPrime; };
    auto u64Hash = std::hash<u64>();
    auto strHash = std::hash<std::string>();

    combineHash(u64Hash(static_cast<u64>(m_passes.size())));
    for (auto& [name, pass] : m_passes)
    {
        combineHash(strHash(name));
        combineHash(pass.generateHash());
    }

    return m_hash;
}

} // namespace huedra