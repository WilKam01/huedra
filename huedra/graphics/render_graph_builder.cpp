#include "render_graph_builder.hpp"
#include "core/log.hpp"
#include "graphics/buffer.hpp"
#include "graphics/texture.hpp"

namespace huedra {

RenderGraphBuilder& RenderGraphBuilder::init()
{
    m_passes.clear();
    m_passKeys.clear();
    return *this;
}

RenderGraphBuilder& RenderGraphBuilder::addPass(const std::string& name, const RenderPassBuilder& pass)
{
    if (m_passes.contains(name))
    {
        log(LogLevel::WARNING, "Could not add render pass, {} already exists", name.c_str());
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
    m_passKeys.push_back(name);
    return *this;
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