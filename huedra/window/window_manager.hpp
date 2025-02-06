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
    friend class Win32Window;
#endif
#ifdef VULKAN
    friend class VulkanContext;
#endif
public:
    WindowManager() = default;
    ~WindowManager() = default;

    void init();
    bool update();
    void cleanup();

    Ref<Window> getFocusedWindow() { return Ref<Window>(p_focusedWindow); }

    Ref<Window> addWindow(const std::string& title, const WindowInput& input, Ref<Window> parent = nullptr);

private:
    void setMousePos(ivec2 pos);
    void setMouseHidden(bool hidden);

    void setFocusedWindow(Window* window) { p_focusedWindow = window; }

    // Create platform specific window
    Window* createWindow(const std::string& title, const WindowInput& input);

    std::vector<Window*> m_windows;
    Window* p_focusedWindow{nullptr};
};

} // namespace huedra