#include "window.hpp"

#include "core/log.hpp"

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

namespace huedra {

bool WindowWayland::init(const std::string& title, const WindowInput& input, wl_shm* wlSharedMemory,
                         wl_compositor* wlCompositor, xdg_wm_base* xdgBase,
                         zxdg_decoration_manager_v1* zxdgDecorationManager)
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

    m_wlSharedMemory = wlSharedMemory;
    m_wlSurface = wl_compositor_create_surface(wlCompositor);
    if (!m_wlSurface)
    {
        log(LogLevel::ERR, "Could not create wayland surface!");
    }

    m_xdgSurface = xdg_wm_base_get_xdg_surface(xdgBase, m_wlSurface);
    if (!m_xdgSurface)
    {
        log(LogLevel::ERR, "Could not get xdg surface!");
    }
    xdg_surface_add_listener(m_xdgSurface, &surfaceListener, this);

    m_xdgToplevel = xdg_surface_get_toplevel(m_xdgSurface);
    if (!m_xdgToplevel)
    {
        log(LogLevel::ERR, "Could not get xdg toplevel!");
    }
    xdg_toplevel_add_listener(m_xdgToplevel, &toplevelListener, this);

    xdg_toplevel_set_title(m_xdgToplevel, title.c_str());
    xdg_toplevel_set_app_id(m_xdgToplevel, title.c_str());

    // Add standard window decoration
    m_zxdgToplevelDecoration = zxdg_decoration_manager_v1_get_toplevel_decoration(zxdgDecorationManager, m_xdgToplevel);
    zxdg_toplevel_decoration_v1_set_mode(m_zxdgToplevelDecoration, ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);

    wl_surface_commit(m_wlSurface);

    return true;
}

void WindowWayland::cleanup()
{
    xdg_toplevel_destroy(m_xdgToplevel);
    xdg_surface_destroy(m_xdgSurface);
    wl_surface_destroy(m_wlSurface);
}

bool WindowWayland::update() { return !m_shouldClose; }

void WindowWayland::setTitle(const std::string& title)
{
    xdg_toplevel_set_title(m_xdgToplevel, title.c_str());
    xdg_toplevel_set_app_id(m_xdgToplevel, title.c_str());
    wl_surface_commit(m_wlSurface);
}

void WindowWayland::setResolution(u32 width, u32 height)
{
    WindowRect rect = getRect();
    rect.width = width;
    rect.height = height;
    rect.screenWidth = width;
    rect.screenHeight = height;

    updateResolution(width, height, width, height);

    resize();
    wl_surface_commit(m_wlSurface);
}

void WindowWayland::setPosition(i32 x, i32 y)
{
    log(LogLevel::WARNING, "Wayland windows do not support global positioning, input position ignored.");
}

void WindowWayland::handleSurfaceConfigure(void* data, xdg_surface* shellSurface, u32 serial)
{
    xdg_surface_ack_configure(shellSurface, serial);
    log(LogLevel::D_INFO, "Got handleShellSurfaceConfigure call!");

    static_cast<WindowWayland*>(data)->resize();
}

void WindowWayland::handleToplevelConfigure(void* data, xdg_toplevel* toplevel, i32 width, i32 height, wl_array* states)
{
    log(LogLevel::D_INFO, "Got handleToplevelConfigure call!");
    if (width != 0 && height != 0)
    {
        log(LogLevel::D_INFO, "Got new width and height ({}, {})!", width, height);
        static_cast<WindowWayland*>(data)->updateResolution(width, height, width, height);
        static_cast<WindowWayland*>(data)->resize();
    }
}

void WindowWayland::handleToplevelClose(void* data, xdg_toplevel* toplevel)
{
    log(LogLevel::D_INFO, "Got handleToplevelClose call!");
    static_cast<WindowWayland*>(data)->m_shouldClose = true;
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

    wl_surface_attach(m_wlSurface, buffer, 0, 0);
    wl_surface_damage_buffer(m_wlSurface, 0, 0, rect.screenWidth, rect.screenHeight);
    wl_surface_commit(m_wlSurface);

    munmap(pixelData, size);
    close(fileDesc);
    wl_shm_pool_destroy(pool);
}

} // namespace huedra