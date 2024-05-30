#pragma once

#include "window/window.hpp"

#include <windows.h>

namespace huedra {

class Win32Window : public Window
{
public:
    Win32Window() = default;
    ~Win32Window() = default;

    bool init(const std::string& title, const WindowInput& input, HINSTANCE instance);
    void cleanup() override;
    bool update() override;

private:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    HWND m_handle;
};

} // namespace huedra