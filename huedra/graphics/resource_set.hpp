#pragma once

#include "graphics/pipeline.hpp"

namespace huedra {

// TODO: Potentially make it possible for using the same set on multiple pipelines

class ResourceSet
{
public:
    ResourceSet() = default;
    virtual ~ResourceSet() = default;

    void init(Pipeline* pipeline, u32 setIndex);
    virtual void cleanup() = 0;

    virtual void assignBuffer(Ref<Buffer> buffer, u32 binding) = 0;

    Ref<Pipeline> getPipeline() { return Ref<Pipeline>(p_pipeline); }
    u32 getSetIndex() { return m_setIndex; }

private:
    Pipeline* p_pipeline;
    u32 m_setIndex;
};

} // namespace huedra
