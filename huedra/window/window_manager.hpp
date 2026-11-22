#pragma once

#include "huedra/core/input/mouse.hpp"
#include "huedra/core/references/reference_counter.hpp"
#include "huedra/core/types.hpp"
#include "huedra/math/vec2.hpp"
#include "huedra/window/window.hpp"

namespace huedra {

class WindowManager
{
    friend class Input;
#ifdef WIN32
    friend class WindowWin32;
#elif defined(MACOS)
    friend class WindowCocoa;
#endif
#ifdef VULKAN
    friend class VulkanContext;
#endif
#ifdef WAYLAND
    friend struct WaylandConfig;
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

    Ref<Window> addWindow(std::string_view title, const WindowInput& input, Ref<Window> parent = Ref<Window>());

private:
    void setMousePosition(ivec2 pos);
    void setCursor(CursorType cursor);
    void setMouseHidden(bool hidden);

    void setFocusedWindow(Window* window) { m_focusedWindow = window; }

    // Create platform specific window
    Window* createWindow(std::string_view title, const WindowInput& input);

    std::vector<Window*> m_windows;
    Window* m_focusedWindow{nullptr};
};

} // namespace huedra