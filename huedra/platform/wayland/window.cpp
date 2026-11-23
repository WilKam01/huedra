#include "window.hpp"

#include "huedra/core/log.hpp"

namespace huedra {

bool WindowWayland::init(std::string_view title, const WindowInput& input, wl_display* wlDisplay,
                         bool graphicsManagerInitialized, wl_buffer* backgroundBuffer, wl_compositor* wlCompositor,
                         xdg_wm_base* xdgBase, zxdg_decoration_manager_v1* zxdgDecorationManager,
                         wp_viewporter* wpViewporter)
{
    WindowRect rect{};
    rect.width = input.width;
    rect.height = input.height;
    rect.screenWidth = input.width;
    rect.screenHeight = input.height;

    // Global position is not available on wayland
    rect.positionX = 0;
    rect.positionY = 0;
    rect.screenPositionX = 0;
    rect.screenPositionY = 0;

    if (input.positionX.has_value() || input.positionY.has_value())
    {
        log::func::warn("Wayland windows do not support global positioning, input position ignored.");
    }

    Window::init(title, rect);

    m_display = wlDisplay;
    m_backgroundBuffer = backgroundBuffer;
    m_graphicsManagerInitialized = graphicsManagerInitialized;

    m_mainSurface = wl_compositor_create_surface(wlCompositor);
    if (!m_mainSurface)
    {
        log::func::fatal("Could not create wayland surface!");
    }
    wl_surface_add_listener(m_mainSurface, &wlSurfaceListener, this);

    m_cursorSurface = wl_compositor_create_surface(wlCompositor);
    if (!m_cursorSurface)
    {
        log::func::fatal("Could not create wayland cursor surface!");
    }
    wl_surface_add_listener(m_cursorSurface, &wlSurfaceListener, this);

    m_xdgSurface = xdg_wm_base_get_xdg_surface(xdgBase, m_mainSurface);
    if (!m_xdgSurface)
    {
        log::func::fatal("Could not get xdg surface!");
    }
    xdg_surface_add_listener(m_xdgSurface, &xdgSurfaceListener, this);

    m_xdgToplevel = xdg_surface_get_toplevel(m_xdgSurface);
    if (!m_xdgToplevel)
    {
        log::func::fatal("Could not get xdg toplevel!");
    }
    xdg_toplevel_add_listener(m_xdgToplevel, &xdgToplevelListener, this);

    m_viewport = wp_viewporter_get_viewport(wpViewporter, m_mainSurface);
    if (!m_viewport)
    {
        log::func::fatal("Could not get wayland viewport!");
    }

    xdg_toplevel_set_title(m_xdgToplevel, title.data());
    xdg_toplevel_set_app_id(m_xdgToplevel, title.data());

    // Add standard window decoration
    m_zxdgToplevelDecoration = zxdg_decoration_manager_v1_get_toplevel_decoration(zxdgDecorationManager, m_xdgToplevel);
    zxdg_toplevel_decoration_v1_set_mode(m_zxdgToplevelDecoration, ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);

    wl_surface_commit(m_mainSurface);

    return true;
}

void WindowWayland::cleanup()
{
    wp_viewport_destroy(m_viewport);
    xdg_toplevel_destroy(m_xdgToplevel);
    xdg_surface_destroy(m_xdgSurface);
    wl_surface_destroy(m_cursorSurface);
    wl_surface_destroy(m_mainSurface);
    Window::cleanup();
}

bool WindowWayland::update() { return !shouldClose(); }

bool WindowWayland::isWithinBounds(ivec2 position, i32 margin) const
{
    return position.x > 0 && position.y > 0 && position.x <= getRect().width && position.y <= getRect().height;
}

bool WindowWayland::isWithinScreenBounds(ivec2 position, i32 margin) const
{
    return position.x > 0 && position.y > 0 && position.x <= getRect().screenWidth &&
           position.y <= getRect().screenHeight;
}

ivec2 WindowWayland::getRelativePosition(ivec2 position) const
{
    if (m_isMouseFocused)
    {
        return position;
    }
    return ivec2(0);
}

ivec2 WindowWayland::getRelativeScreenPosition(ivec2 position) const
{
    if (m_isMouseFocused)
    {
        return position;
    }
    return ivec2(0);
}

void WindowWayland::setTitle(std::string_view title)
{
    xdg_toplevel_set_title(m_xdgToplevel, title.data());
    xdg_toplevel_set_app_id(m_xdgToplevel, title.data());
    updateTitle(title);
}

void WindowWayland::setResolution(uvec2 resolution)
{
    updateResolution(resolution.x, resolution.y, resolution.x, resolution.y);
    resize();
}

void WindowWayland::setPosition(ivec2 position)
{
    log::func::warn("Wayland windows do not support global positioning, input position ignored.");
}

void WindowWayland::handleSurfaceConfigure(void* data, xdg_surface* shellSurface, u32 serial)
{
    xdg_surface_ack_configure(shellSurface, serial);
    static_cast<WindowWayland*>(data)->resize();
}

void WindowWayland::handleToplevelConfigure(void* data, xdg_toplevel* toplevel, i32 width, i32 height, wl_array* states)
{
    auto* window = static_cast<WindowWayland*>(data);
    if (width != 0 && height != 0)
    {
        window->updateResolution(width, height, width, height);

        // TODO: This is not the optimal solution due to it not being active when not in focus, find better solution
        // Assume minimized until proven wrong
        /*window->updateMinimized(true);
        for (u32* state = std::bit_cast<u32*>(states->data);
             states->size != 0 &&
             std::bit_cast<const char*>(state) < (std::bit_cast<const char*>(states->data) + states->size);
             state++)
        {
            if (*state == XDG_TOPLEVEL_STATE_ACTIVATED)
            {
                window->updateMinimized(false);
            }
        }*/
    }
}

void WindowWayland::handleToplevelClose(void* data, xdg_toplevel* toplevel)
{
    static_cast<WindowWayland*>(data)->setShouldClose();
}

void WindowWayland::resize()
{
    if (m_lastScreenSize == getScreenSize())
    {
        return;
    }
    m_lastScreenSize = getScreenSize();

    // Let graphicsManager take care of resizing window
    // Will throw error otherwise, in the case of Vulkan
    if (m_graphicsManagerInitialized)
    {
        return;
    }

    WindowRect rect = getRect();
    wl_surface_attach(m_mainSurface, m_backgroundBuffer, 0, 0);
    wp_viewport_set_destination(m_viewport, rect.screenWidth, rect.screenHeight);
    wl_surface_damage_buffer(m_mainSurface, 0, 0, 1, 1);
    wl_surface_commit(m_mainSurface);
}

} // namespace huedra