#pragma once

#include "core/types.hpp"
#include "graphics/swapchain.hpp"

namespace huedra {

class GraphicalContext
{

public:
    GraphicalContext() = default;
    virtual ~GraphicalContext() = default;

    virtual void init() = 0;
    virtual void cleanup() = 0;

    virtual Swapchain* createSwapchain(Window* window) = 0;
    virtual void removeSwapchain(size_t index) = 0;
    virtual void recordGraphicsCommands(u32 swapchainIndex, u32 imageIndex) = 0;

private:
};

} // namespace huedra