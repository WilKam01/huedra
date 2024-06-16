#pragma once

#include "window/window.hpp"

namespace huedra {

class Swapchain
{
public:
    Swapchain() = default;
    virtual ~Swapchain() = default;

    void init(Window* window);
    virtual void cleanup() = 0;

    virtual bool graphicsReady() = 0;
    virtual std::optional<u32> acquireNextImage() = 0;
    virtual void submitGraphicsQueue(u32 imageIndex) = 0;
    virtual bool present(u32 imageIndex) = 0;

protected:
    Window* getWindow() const { return p_window; }

    static constexpr u32 MAX_FRAMES_IN_FLIGHT = 2;

private:
    Window* p_window;
};

} // namespace huedra
