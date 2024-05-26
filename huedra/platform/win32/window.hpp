#pragma once

#include "window/window.hpp"

#include <windows.h>

namespace huedra {

class Win32Window : public Window
{
public:
    Win32Window() = default;
    ~Win32Window() = default;

    bool init(const std::string& title, Vector2i rect, HINSTANCE instance);
    void cleanup() override;
    bool update() override;

private:
    HWND m_window;
};

} // namespace huedra