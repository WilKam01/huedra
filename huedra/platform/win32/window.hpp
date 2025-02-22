#pragma once

#include "core/input/keys.hpp"
#include "window/window.hpp"

#include <windows.h>

namespace huedra {

class Win32Window : public Window
{
public:
    Win32Window() = default;
    ~Win32Window() override = default;

    Win32Window(const Win32Window& rhs) = default;
    Win32Window& operator=(const Win32Window& rhs) = default;
    Win32Window(Win32Window&& rhs) = default;
    Win32Window& operator=(Win32Window&& rhs) = default;

    bool init(const std::string& title, const WindowInput& input, HINSTANCE instance);
    void cleanup() override;
    bool update() override;

    void setTitle(const std::string& title) override;
    void setResolution(u32 width, u32 height) override;
    void setPos(i32 x, i32 y) override;

    HWND getHandle() { return m_handle; }

    static LRESULT CALLBACK windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    static Keys convertKey(u32 code);

    HWND m_handle{nullptr};
};

} // namespace huedra