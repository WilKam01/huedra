#include "os_manager.hpp"
#include "core/log.hpp"

#ifdef WIN32
#include "platform/win32/window.hpp"
#elif defined(WAYLAND)
#include "platform/wayland/window.hpp"
#endif

namespace huedra {

VkSurfaceKHR createSurface(Instance& instance, Window* window)
{
    VkSurfaceKHR surface{nullptr};

#ifdef WIN32
    auto* win = static_cast<WindowWin32*>(window);
    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hinstance = GetModuleHandle(nullptr);
    createInfo.hwnd = win->getHandle();

    if (vkCreateWin32SurfaceKHR(instance.get(), &createInfo, nullptr, &surface) != VK_SUCCESS)
    {
        log(LogLevel::ERR, "Failed to create Win32 surface!");
    }
#elif defined(WAYLAND)
    auto* win = static_cast<WindowWayland*>(window);
    VkWaylandSurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    createInfo.flags = 0;
    createInfo.display = win->getDisplay();
    createInfo.surface = win->getSurface();

    if (vkCreateWaylandSurfaceKHR(instance.get(), &createInfo, nullptr, &surface) != VK_SUCCESS)
    {
        log(LogLevel::ERR, "Failed to create Wayland surface!");
    }
#endif

    return surface;
}

} // namespace huedra