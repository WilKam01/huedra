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

    bool init(const std::string& title, const WindowInput& input, bool graphicsManagersInitialized,
              wl_display* wlDisplay, wl_shm* wlSharedMemory, wl_compositor* wlCompositor, xdg_wm_base* xdgBase,
              zxdg_decoration_manager_v1* zxdgDecorationManager);
    void cleanup() override;
    bool update() override;

    bool isWithinBounds(ivec2 position, i32 margin = 0) const override;
    bool isWithinScreenBounds(ivec2 position, i32 margin = 0) const override;
    ivec2 getRelativePosition(ivec2 position) const override;
    ivec2 getRelativeScreenPosition(ivec2 position) const override;

    void setTitle(const std::string& title) override;
    void setResolution(uvec2 resolution) override;
    void setPosition(ivec2 position) override;

    void setIsMouseFocused(bool isMouseFocused) { m_isMouseFocused = isMouseFocused; }
    void setLastPointerSerial(u32 serial) { m_lastPointerSerial = serial; }

    wl_display* getDisplay() { return m_display; }
    wl_surface* getSurface() { return m_mainSurface; }
    wl_surface* getCursorSurface() { return m_cursorSurface; }
    bool isMouseFocused() const { return m_isMouseFocused; }
    u32 getLastPointerSerial() const { return m_lastPointerSerial; }

private:
    static void handleSurfaceConfigure(void* data, xdg_surface* shellSurface, u32 serial);
    static void handleToplevelConfigure(void* data, xdg_toplevel* toplevel, i32 width, i32 height, wl_array* states);
    static void handleToplevelClose(void* data, xdg_toplevel* toplevel);

    void resize();

    bool m_isMouseFocused{false};
    u32 m_lastPointerSerial{0};
    bool m_isGraphicsManagerInitialized{false};

    // Per window
    wl_surface* m_mainSurface{nullptr};
    xdg_surface* m_xdgSurface{nullptr};
    xdg_toplevel* m_xdgToplevel{nullptr};
    zxdg_toplevel_decoration_v1* m_zxdgToplevelDecoration{nullptr};
    wl_surface* m_cursorSurface{nullptr};

    // References
    wl_display* m_display{nullptr};
    wl_shm* m_wlSharedMemory{nullptr};

    static constexpr wl_surface_listener wlSurfaceListener{};
    static constexpr xdg_surface_listener xdgSurfaceListener{.configure = handleSurfaceConfigure};
    static constexpr xdg_toplevel_listener xdgToplevelListener{.configure = handleToplevelConfigure,
                                                               .close = handleToplevelClose};
};

} // namespace huedra