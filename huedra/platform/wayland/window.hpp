#pragma once

#include "core/input/keys.hpp"
#include "window/window.hpp"

#include <xdg-decoration.h>
#include <xdg-shell.h>

namespace huedra {

class WindowWayland : public Window
{
public:
    WindowWayland() = default;
    ~WindowWayland() override = default;

    WindowWayland(const WindowWayland& rhs) = default;
    WindowWayland& operator=(const WindowWayland& rhs) = default;
    WindowWayland(WindowWayland&& rhs) = default;
    WindowWayland& operator=(WindowWayland&& rhs) = default;

    bool init(const std::string& title, const WindowInput& input, wl_shm* wlSharedMemory, wl_compositor* wlCompositor,
              xdg_wm_base* xdgBase, zxdg_decoration_manager_v1* zxdgDecorationManager);
    void cleanup() override;
    bool update() override;

    void setTitle(const std::string& title) override;
    void setResolution(u32 width, u32 height) override;
    void setPosition(i32 x, i32 y) override;

private:
    static void handleSurfaceConfigure(void* data, xdg_surface* shellSurface, u32 serial);
    static void handleToplevelConfigure(void* data, xdg_toplevel* toplevel, i32 width, i32 height, wl_array* states);
    static void handleToplevelClose(void* data, xdg_toplevel* toplevel);

    void resize();

    // Per window
    wl_surface* m_wlSurface{nullptr};
    xdg_surface* m_xdgSurface{nullptr};
    xdg_toplevel* m_xdgToplevel{nullptr};
    zxdg_toplevel_decoration_v1* m_zxdgToplevelDecoration{nullptr};
    bool m_shouldClose{false};

    // References
    wl_shm* m_wlSharedMemory{nullptr};

    static constexpr xdg_surface_listener surfaceListener{.configure = handleSurfaceConfigure};
    static constexpr xdg_toplevel_listener toplevelListener = {.configure = handleToplevelConfigure,
                                                               .close = handleToplevelClose};
};

} // namespace huedra