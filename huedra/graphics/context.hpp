#pragma once

#include "core/types.hpp"
#include "graphics/buffer.hpp"
#include "graphics/render_graph_builder.hpp"
#include "graphics/resource_set.hpp"
#include "window/window.hpp"

namespace huedra {

class GraphicalContext
{

public:
    GraphicalContext() = default;
    virtual ~GraphicalContext() = default;

    virtual void init() = 0;
    virtual void cleanup() = 0;

    virtual void createSwapchain(Window* window, bool renderDepth) = 0;
    virtual void removeSwapchain(size_t index) = 0;

    virtual Pipeline* createPipeline(const PipelineBuilder& pipelineBuilder) = 0;
    virtual Buffer* createBuffer(BufferType type, BufferUsageFlags usage, u64 size, void* data) = 0;
    virtual ResourceSet* createResourceSet(Pipeline* pipeline, u32 setIndex) = 0;

    virtual void setRenderGraph(RenderGraphBuilder& builder) = 0;
    virtual void render() = 0;

private:
};

} // namespace huedra