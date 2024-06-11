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

    virtual void recreate() = 0;

protected:
    Window* getWindow() const { return p_window; }

private:
    Window* p_window;
};

} // namespace huedra
