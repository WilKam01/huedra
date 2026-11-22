#pragma once

#include "huedra/core/references/ref.hpp"
#include "huedra/core/types.hpp"
#include "huedra/graphics/pipeline_builder.hpp"
#include "huedra/graphics/render_pass_builder.hpp"
#include "huedra/graphics/render_target.hpp"
#include "huedra/math/vec3.hpp"

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
    RenderGraphBuilder& addPass(std::string_view name, const RenderPassBuilder& pass);

    u64 generateHash();

    bool empty() const { return m_passes.empty(); }
    u64 getHash() const { return m_hash; }
    const RenderPassBuilder& getRenderPass(std::string_view key) const { return m_passes.at(key.data()); }
    const std::vector<std::string>& getRenderPassNames() const { return m_passKeys; }

private:
    u64 m_hash{0};

    std::map<std::string, RenderPassBuilder> m_passes;
    std::vector<std::string> m_passKeys; // In insertion order
};

} // namespace huedra