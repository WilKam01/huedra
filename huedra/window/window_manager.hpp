#pragma once

#include "core/references/reference_counter.hpp"
#include "core/types.hpp"
#include "window/window.hpp"

namespace huedra {

class WindowManager
{
#ifdef VULKAN
    friend class VulkanContext;
#endif
public:
    WindowManager() = default;
    ~WindowManager() = default;

    void init();
    bool update();
    void cleanup();

    Ref<Window> addWindow(const std::string& title, const WindowInput& input, Ref<Window> parent = nullptr);

private:
    // Create platform specific window
    Window* createWindow(const std::string& title, const WindowInput& input);

    std::vector<Window*> m_windows;
};

} // namespace huedra