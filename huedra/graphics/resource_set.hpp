#pragma once

#include "core/references/ref.hpp"
#include "graphics/buffer.hpp"
#include "graphics/pipeline_builder.hpp"
#include "graphics/texture.hpp"

namespace huedra {

class ResourceSet
{
public:
    ResourceSet();
    virtual ~ResourceSet();

    void init(u32 setIndex, const std::vector<ResourceBinding>& bindings);
    virtual void cleanup() = 0;

    virtual void assignBuffer(Ref<Buffer> buffer, u32 binding) = 0;
    virtual void assignTexture(Ref<Texture> texture, u32 binding) = 0;

    u32 getSetIndex() { return m_setIndex; }

    bool isCompatible(const PipelineBuilder& builder) const;

private:
    u32 m_setIndex;
    std::vector<ResourceBinding> m_bindings;
};

} // namespace huedra
