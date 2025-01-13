#pragma once

#include "core/types.hpp"
#include "graphics/render_graph_builder.hpp"
#include "resources/texture/data.hpp"
#include "window/window.hpp"

namespace huedra {

class Buffer;
class Texture;

class GraphicalContext
{

public:
    GraphicalContext() = default;
    virtual ~GraphicalContext() = default;

    virtual void init() = 0;
    virtual void cleanup() = 0;

    virtual void createSwapchain(Window* window, bool renderDepth) = 0;
    virtual void removeSwapchain(size_t index) = 0;

    virtual Buffer* createBuffer(BufferType type, BufferUsageFlags usage, u64 size, void* data) = 0;
    virtual void readBuffer(u64 id, u64 size, void* data) = 0;
    virtual void writeToBuffer(u64 id, u64 size, void* data) = 0;

    virtual Texture* createTexture(TextureData textureData) = 0;

    virtual void prepareSwapchains() = 0;
    virtual void setRenderGraph(RenderGraphBuilder& builder) = 0;
    virtual void render() = 0;

private:
};

} // namespace huedra