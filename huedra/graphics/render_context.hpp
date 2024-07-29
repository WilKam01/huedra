#pragma once

#include "core/references/ref.hpp"
#include "graphics/buffer.hpp"

namespace huedra {

class ResourceSet;

class RenderContext
{
public:
    RenderContext() = default;
    virtual ~RenderContext() = default;

    virtual void bindVertexBuffers(std::vector<Ref<Buffer>> buffers) = 0;
    virtual void bindIndexBuffer(Ref<Buffer> buffer) = 0;
    virtual void bindResourceSets(std::vector<Ref<ResourceSet>> resourceSets) = 0;
    virtual void bindResourceSet(Ref<ResourceSet> resourceSet) = 0;

    virtual void pushConstants(ShaderStageFlags shaderStage, u32 size, void* data) = 0;

    virtual void draw(u32 vertexCount, u32 instanceCount, u32 vertexOffset, u32 instanceOffset) = 0;
    virtual void drawIndexed(u32 indexCount, u32 instanceCount, u32 indexOffset, u32 instanceOffset) = 0;

private:
};

} // namespace huedra