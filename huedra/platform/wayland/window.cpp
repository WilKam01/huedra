#include "window.hpp"

#include "core/log.hpp"

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

namespace huedra {

bool WindowWayland::init(const std::string& title, const WindowInput& input, bool graphicsManagersInitialized,
                         wl_display* wlDisplay, wl_shm* wlSharedMemory, wl_compositor* wlCompositor,
                         xdg_wm_base* xdgBase, zxdg_decoration_manager_v1* zxdgDecorationManager)
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
        log(LogLevel::WARNING, "Wayland windows do not support global positioning, input position ignored.");
    }

    Window::init(title, rect);

    m_isGraphicsManagerInitialized = graphicsManagersInitialized;
    m_display = wlDisplay;
    m_wlSharedMemory = wlSharedMemory;

    m_mainSurface = wl_compositor_create_surface(wlCompositor);
    if (!m_mainSurface)
    {
        log(LogLevel::ERR, "Could not create wayland surface!");
    }
    wl_surface_add_listener(m_mainSurface, &wlSurfaceListener, this);

    m_cursorSurface = wl_compositor_create_surface(wlCompositor);
    if (!m_cursorSurface)
    {
        log(LogLevel::ERR, "Could not create wayland cursor surface!");
    }
    wl_surface_add_listener(m_cursorSurface, &wlSurfaceListener, this);

    m_xdgSurface = xdg_wm_base_get_xdg_surface(xdgBase, m_mainSurface);
    if (!m_xdgSurface)
    {
        log(LogLevel::ERR, "Could not get xdg surface!");
    }
    xdg_surface_add_listener(m_xdgSurface, &xdgSurfaceListener, this);

    m_xdgToplevel = xdg_surface_get_toplevel(m_xdgSurface);
    if (!m_xdgToplevel)
    {
        log(LogLevel::ERR, "Could not get xdg toplevel!");
    }
    xdg_toplevel_add_listener(m_xdgToplevel, &xdgToplevelListener, this);

    xdg_toplevel_set_title(m_xdgToplevel, title.c_str());
    xdg_toplevel_set_app_id(m_xdgToplevel, title.c_str());

    // Add standard window decoration
    m_zxdgToplevelDecoration = zxdg_decoration_manager_v1_get_toplevel_decoration(zxdgDecorationManager, m_xdgToplevel);
    zxdg_toplevel_decoration_v1_set_mode(m_zxdgToplevelDecoration, ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);

    wl_surface_commit(m_mainSurface);

    return true;
}

void WindowWayland::cleanup()
{
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

void WindowWayland::setTitle(const std::string& title)
{
    xdg_toplevel_set_title(m_xdgToplevel, title.c_str());
    xdg_toplevel_set_app_id(m_xdgToplevel, title.c_str());
    updateTitle(title);
}

void WindowWayland::setResolution(uvec2 resolution)
{
    updateResolution(resolution.x, resolution.y, resolution.x, resolution.y);
    resize();
}

void WindowWayland::setPosition(ivec2 position)
{
    log(LogLevel::WARNING, "Wayland windows do not support global positioning, input position ignored.");
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
        window->resize();
    }

    // TODO: Figure out different way of checking minimized
    // window->updateMinimized(true);
    char* state = nullptr;
    for (u32* state = static_cast<u32*>(states->data); state < static_cast<u32*>(states->data) + states->size; ++state)
    {
        if (*state == XDG_TOPLEVEL_STATE_ACTIVATED)
        {
            window->updateMinimized(false);
        }
    }
}

void WindowWayland::handleToplevelClose(void* data, xdg_toplevel* toplevel)
{
    static_cast<WindowWayland*>(data)->setShouldClose();
}

void WindowWayland::resize()
{
    WindowRect rect = getRect();
    i32 stride = rect.screenWidth * 4; // RGBA components for each scanline
    i32 size = stride * rect.screenHeight;

    i32 fileDesc = memfd_create("wayland-shm-buffer", MFD_CLOEXEC);
    if (fileDesc < 0)
    {
        log(LogLevel::ERR, "Could not create wayland-shm-buffer in RAM!");
    }

    if (ftruncate(fileDesc, size) < 0)
    {
        close(fileDesc);
        log(LogLevel::ERR, "Could not trunctate wayland-shm-buffer file to desired size ({})", size);
    }

    u32* pixelData = std::bit_cast<u32*>(mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fileDesc, 0));
    for (u32 i = 0; i < rect.screenWidth * rect.screenHeight; ++i)
    {
        pixelData[i] = 0xff222222;
    }

    wl_shm_pool* pool = wl_shm_create_pool(m_wlSharedMemory, fileDesc, size);
    wl_buffer* buffer =
        wl_shm_pool_create_buffer(pool, 0, rect.screenWidth, rect.screenHeight, stride, WL_SHM_FORMAT_XRGB8888);

    wl_surface_attach(m_mainSurface, buffer, 0, 0);
    wl_surface_damage_buffer(m_mainSurface, 0, 0, rect.screenWidth, rect.screenHeight);

    // When graphics manager is initalized, let it handle surface rendering
    if (!m_isGraphicsManagerInitialized)
    {
        wl_surface_commit(m_mainSurface);
    }

    munmap(pixelData, size);
    close(fileDesc);
    wl_shm_pool_destroy(pool);
}

} // namespace huedra