#pragma once

#include "core/references/ref.hpp"
#include "graphics/pipeline_data.hpp"

namespace huedra {

class ResourceSet;
class Buffer;
class Texture;

class RenderContext
{
public:
    RenderContext() = default;
    virtual ~RenderContext() = default;

    virtual void bindVertexBuffers(std::vector<Ref<Buffer>> buffers) = 0;
    virtual void bindIndexBuffer(Ref<Buffer> buffer) = 0;
    virtual void bindBuffer(Ref<Buffer> buffer, u32 set, u32 binding) = 0;
    virtual void bindTexture(Ref<Texture> texture, u32 set, u32 binding) = 0;

    virtual void pushConstants(ShaderStageFlags shaderStage, u32 size, void* data) = 0;

    virtual void draw(u32 vertexCount, u32 instanceCount, u32 vertexOffset, u32 instanceOffset) = 0;
    virtual void drawIndexed(u32 indexCount, u32 instanceCount, u32 indexOffset, u32 instanceOffset) = 0;

private:
};

} // namespace huedra