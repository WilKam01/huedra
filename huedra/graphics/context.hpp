#pragma once

#include "core/types.hpp"
#include "graphics/render_pass.hpp"
#include "window/window.hpp"

namespace huedra {

class GraphicalContext
{

public:
    GraphicalContext() = default;
    virtual ~GraphicalContext() = default;

    virtual void init() = 0;
    virtual void cleanup() = 0;

    virtual void createSwapchain(Window* window) = 0;
    virtual void removeSwapchain(size_t index) = 0;

    virtual void prepareRendering() = 0;
    virtual void recordGraphicsCommands(RenderPass& renderPass) = 0;
    virtual void submitGraphicsQueue() = 0;
    virtual void presentSwapchains() = 0;

    virtual u32 getFrameIndex() = 0;

private:
};

} // namespace huedra