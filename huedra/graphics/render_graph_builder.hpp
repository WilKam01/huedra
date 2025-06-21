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

class RenderGraphBuilder
{
public:
    RenderGraphBuilder() = default;
    ~RenderGraphBuilder() = default;

    RenderGraphBuilder(const RenderGraphBuilder& rhs) = default;
    RenderGraphBuilder& operator=(const RenderGraphBuilder& rhs) = default;
    RenderGraphBuilder(RenderGraphBuilder&& rhs) = default;
    RenderGraphBuilder& operator=(RenderGraphBuilder&& rhs) = default;

    RenderGraphBuilder& init();
    RenderGraphBuilder& addPass(const std::string& name, const RenderPassBuilder& pass);

    u64 generateHash();

    bool empty() const { return m_passes.empty(); }
    u64 getHash() const { return m_hash; }
    const RenderPassBuilder& getRenderPass(const std::string& key) const { return m_passes.at(key); }
    const std::vector<std::string>& getRenderPassNames() const { return m_passKeys; }

private:
    u64 m_hash{0};

    std::map<std::string, RenderPassBuilder> m_passes;
    std::vector<std::string> m_passKeys; // In insertion order
};

} // namespace huedra