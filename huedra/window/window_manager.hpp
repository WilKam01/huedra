#pragma once

#include "core/types.hpp"
#include "window/window.hpp"

namespace huedra {

class WindowManager
{
public:
    WindowManager() = default;
    ~WindowManager() = default;

    void init();
    bool update();
    void cleanup();

    Window* createWindow(const std::string& title, const WindowInput& input);

private:
    std::vector<Window*> m_windows;
};

} // namespace huedra