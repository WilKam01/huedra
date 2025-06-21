#pragma once

#include "core/references/ref.hpp"
#include "graphics/pipeline_data.hpp"
#include "math/vec3.hpp"

namespace huedra {

class ResourceSet;
class Buffer;
class Texture;

class RenderContext
{
public:
    RenderContext() = default;
    virtual ~RenderContext() = default;

    RenderContext(const RenderContext& rhs) = delete;
    RenderContext& operator=(const RenderContext& rhs) = delete;
    RenderContext(RenderContext&& rhs) = delete;
    RenderContext& operator=(RenderContext&& rhs) = delete;

    virtual void bindVertexBuffers(std::vector<Ref<Buffer>> buffers) = 0;
    virtual void bindIndexBuffer(Ref<Buffer> buffer) = 0;

    virtual void bindBuffer(Ref<Buffer> buffer, std::string_view name) = 0;
    virtual void bindTexture(Ref<Texture> texture, std::string_view name) = 0;
    virtual void bindSampler(const SamplerSettings& sampler, std::string_view name) = 0;
    virtual void setParameter(void* data, u32 size, std::string_view name) = 0;

    virtual void draw(u32 vertexCount, u32 instanceCount, u32 vertexOffset, u32 instanceOffset) = 0;
    virtual void drawIndexed(u32 indexCount, u32 instanceCount, u32 indexOffset, u32 instanceOffset) = 0;
    virtual void dispatchGroups(u32 groupX, u32 groupY, u32 groupZ) = 0;
    // Automatically translates based on what getComputeThreadsPerGroup is defined as
    // Input is how many calculations should be done in dispatch
    virtual void dispatch(u32 x, u32 y, u32 z) = 0;

    virtual uvec3 getComputeThreadsPerGroup() = 0;

private:
};

} // namespace huedra