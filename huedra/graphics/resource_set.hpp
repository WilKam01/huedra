#pragma once

#include "core/references/ref.hpp"
#include "graphics/buffer.hpp"

namespace huedra {

class ResourceSet
{
public:
    ResourceSet();
    virtual ~ResourceSet();

    void init(u32 setIndex);
    virtual void cleanup() = 0;

    virtual void assignBuffer(Ref<Buffer> buffer, u32 binding) = 0;

    u32 getSetIndex() { return m_setIndex; }

private:
    u32 m_setIndex;
};

} // namespace huedra
