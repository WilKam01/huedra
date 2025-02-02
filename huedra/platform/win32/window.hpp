#pragma once

#include "core/input/keys.hpp"
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

    void setTitle(const std::string& title) override;
    void setResolution(u32 width, u32 height) override;
    void setPos(i32 x, i32 y) override;

    HWND getHandle() { return m_handle; }

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    static Keys convertKey(i64 code);

    HWND m_handle;
};

} // namespace huedra