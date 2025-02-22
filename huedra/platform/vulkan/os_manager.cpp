#include "os_manager.hpp"
#include "core/log.hpp"

#ifdef WIN32
#include "platform/win32/window.hpp"
#endif

namespace huedra {

VkSurfaceKHR createSurface(Instance& instance, Window* window)
{
    VkSurfaceKHR surface{nullptr};

#ifdef WIN32
    auto* win = static_cast<Win32Window*>(window);
    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hinstance = GetModuleHandle(nullptr);
    createInfo.hwnd = win->getHandle();

    if (vkCreateWin32SurfaceKHR(instance.get(), &createInfo, nullptr, &surface) != VK_SUCCESS)
    {
        log(LogLevel::ERR, "Failed to create Win32 surface!");
    }
#endif

    return surface;
}

} // namespace huedra