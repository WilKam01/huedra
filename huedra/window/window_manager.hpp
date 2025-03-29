#pragma once

#include "core/input/mouse.hpp"
#include "core/references/reference_counter.hpp"
#include "core/types.hpp"
#include "math/vec2.hpp"
#include "window/window.hpp"

namespace huedra {

class WindowManager
{
    friend class Input;
#ifdef WIN32
    friend class WindowWin32;
#endif
#ifdef VULKAN
    friend class VulkanContext;
#endif
public:
    WindowManager() = default;
    ~WindowManager() = default;

    WindowManager(const WindowManager& rhs) = default;
    WindowManager& operator=(const WindowManager& rhs) = default;
    WindowManager(WindowManager&& rhs) = default;
    WindowManager& operator=(WindowManager&& rhs) = default;

    void init();
    bool update();
    void cleanup();

    Ref<Window> getFocusedWindow() { return Ref<Window>(m_focusedWindow); }

    Ref<Window> addWindow(const std::string& title, const WindowInput& input, Ref<Window> parent = Ref<Window>());

private:
    void setMousePos(ivec2 pos);
    void setCursor(CursorType cursor);
    void setMouseHidden(bool hidden);

    void setFocusedWindow(Window* window) { m_focusedWindow = window; }

    // Create platform specific window
    Window* createWindow(const std::string& title, const WindowInput& input);

    std::vector<Window*> m_windows;
    Window* m_focusedWindow{nullptr};
};

} // namespace huedra