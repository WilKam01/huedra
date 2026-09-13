#include "window.hpp"
#include "core/log.hpp"

namespace huedra {

bool WindowX11::init(const std::string& title, const WindowInput& input, xcb_connection_t* xcbConnection,
                     xcb_screen_t* xcbScreen, xcb_intern_atom_reply_t* wmProtoReply,
                     xcb_intern_atom_reply_t* wmDeleteReply)
{
    m_xcbConnection = xcbConnection;
    m_window = xcb_generate_id(m_xcbConnection);

    xcb_void_cookie_t cookie;
    xcb_generic_error_t* error;

    u32 mask = XCB_CW_BACK_PIXEL | XCB_CW_BIT_GRAVITY | XCB_CW_WIN_GRAVITY | XCB_CW_EVENT_MASK;
    std::vector<u32> list = {0xff222222, XCB_GRAVITY_STATIC, XCB_GRAVITY_STATIC,
                             XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_BUTTON_PRESS |
                                 XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION |
                                 XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_FOCUS_CHANGE |
                                 XCB_EVENT_MASK_PROPERTY_CHANGE};
    cookie =
        xcb_create_window_checked(m_xcbConnection, XCB_COPY_FROM_PARENT, m_window, xcbScreen->root,
                                  input.positionX.value_or(0), input.positionY.value_or(0), input.width, input.height,
                                  0, XCB_WINDOW_CLASS_INPUT_OUTPUT, xcbScreen->root_visual, mask, list.data());
    error = xcb_request_check(m_xcbConnection, cookie);
    if (error != nullptr)
    {
        log(LogLevel::ERR, "Failed to create XCB window: {}", getXcbError(error));
        return false;
    }

    cookie = xcb_map_window_checked(m_xcbConnection, m_window);
    error = xcb_request_check(m_xcbConnection, cookie);
    if (error != nullptr)
    {
        log(LogLevel::ERR, "Failed to map XCB window: {}", getXcbError(error));
        return false;
    }

    cookie = xcb_change_property_checked(m_xcbConnection, XCB_PROP_MODE_REPLACE, m_window, XCB_ATOM_WM_NAME,
                                         XCB_ATOM_STRING, 8, title.length(), title.c_str());
    error = xcb_request_check(m_xcbConnection, cookie);
    if (error != nullptr)
    {
        log(LogLevel::ERR, "Failed to replace name on XCB window: {}", getXcbError(error));
        return false;
    }

    if (wmProtoReply && wmDeleteReply)
    {
        cookie = xcb_change_property_checked(m_xcbConnection, XCB_PROP_MODE_REPLACE, m_window, wmProtoReply->atom, 4,
                                             32, 1, &wmDeleteReply->atom);
        error = xcb_request_check(m_xcbConnection, cookie);
        if (error != nullptr)
        {
            log(LogLevel::ERR, "Failed to set proto and/or delete reply on XCB window: {}", getXcbError(error));
            return false;
        }
    }

    WindowRect rect;
    rect.positionX = input.positionX.value_or(0);
    rect.positionY = input.positionY.value_or(0);
    rect.screenPositionX = input.positionX.value_or(0);
    rect.screenPositionY = input.positionY.value_or(0);
    rect.width = input.width;
    rect.height = input.height;
    rect.screenWidth = input.width;
    rect.screenHeight = input.height;
    Window::init(title, rect);

    return true;
}

void WindowX11::cleanup()
{
    xcb_destroy_window(m_xcbConnection, m_window);
    Window::cleanup();
}

bool WindowX11::update() { return true; }

bool WindowX11::isWithinBounds(ivec2 position, i32 margin) const
{
    WindowRect rect = getRect();
    return position.x >= rect.positionX + margin &&
           position.x <= rect.positionX + static_cast<i32>(rect.width) - margin &&
           position.y >= rect.positionY + margin &&
           position.y <= rect.positionY + static_cast<i32>(rect.height) - margin && !isMinimized();
}

bool WindowX11::isWithinScreenBounds(ivec2 position, i32 margin) const
{
    WindowRect rect = getRect();
    return position.x >= rect.screenPositionX + margin &&
           position.x <= rect.screenPositionX + static_cast<i32>(rect.screenWidth) - margin &&
           position.y >= rect.screenPositionY + margin &&
           position.y <= rect.screenPositionY + static_cast<i32>(rect.screenHeight) - margin && !isMinimized();
}

ivec2 WindowX11::getRelativePosition(ivec2 position) const { return position - getPosition(); }

ivec2 WindowX11::getRelativeScreenPosition(ivec2 position) const { return position - getScreenPosition(); }

void WindowX11::setTitle(const std::string& title)
{
    xcb_void_cookie_t cookie =
        xcb_change_property_checked(m_xcbConnection, XCB_PROP_MODE_REPLACE, m_window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING,
                                    8, title.length(), title.c_str());
    xcb_generic_error_t* error = xcb_request_check(m_xcbConnection, cookie);
    if (error != nullptr)
    {
        log(LogLevel::ERR, "Failed to change window title: {}", getXcbError(error));
    }
    xcb_flush(m_xcbConnection);
    updateTitle(title);
}

void WindowX11::setResolution(uvec2 resolution)
{
    u16 mask = XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT;
    std::vector<u32> list{resolution.x, resolution.y};
    xcb_void_cookie_t cookie = xcb_configure_window_checked(m_xcbConnection, m_window, mask, list.data());
    xcb_generic_error_t* error = xcb_request_check(m_xcbConnection, cookie);
    if (error != nullptr)
    {
        log(LogLevel::ERR, "Failed to change window resolution: {}", getXcbError(error));
    }
    xcb_flush(m_xcbConnection);
}

void WindowX11::setPosition(ivec2 position)
{
    u16 mask = XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y;
    std::vector<u32> list{static_cast<u32>(position.x), static_cast<u32>(position.y)};
    xcb_void_cookie_t cookie = xcb_configure_window_checked(m_xcbConnection, m_window, mask, list.data());
    xcb_generic_error_t* error = xcb_request_check(m_xcbConnection, cookie);
    if (error != nullptr)
    {
        log(LogLevel::ERR, "Failed to change window position: {}", getXcbError(error));
    }
    xcb_flush(m_xcbConnection);
}

std::string WindowX11::getXcbError(xcb_generic_error_t* error)
{
    std::array<std::string_view, 17> errorNames = {
        "XCB_REQUEST",   "XCB_VALUE",     "XCB_WINDOW",   "XCB_PIXMAP", "XCB_ATOM",          "XCB_CURSOR",
        "XCB_FONT",      "XCB_MATCH",     "XCB_DRAWABLE", "XCB_ACCESS", "XCB_ALLOC",         "XCB_COLORMAP",
        "XCB_G_CONTEXT", "XCB_ID_CHOICE", "XCB_NAME",     "XCB_LENGTH", "XCB_IMPLEMENTATION"};

    switch (error->error_code)
    {
    case XCB_REQUEST:
    case XCB_MATCH:
    case XCB_ACCESS:
    case XCB_ALLOC:
    case XCB_NAME:
    case XCB_LENGTH:
    case XCB_IMPLEMENTATION: {
        auto* er = std::bit_cast<xcb_request_error_t*>(error);
        return std::format("{} request error, code: {}, major: {}, minor: {}", errorNames[er->error_code],
                           er->error_code, er->major_opcode, er->minor_opcode);
    }
    break;
    case XCB_VALUE:
    case XCB_WINDOW:
    case XCB_PIXMAP:
    case XCB_ATOM:
    case XCB_CURSOR:
    case XCB_FONT:
    case XCB_DRAWABLE:
    case XCB_COLORMAP:
    case XCB_G_CONTEXT:
    case XCB_ID_CHOICE: {
        auto* er = std::bit_cast<xcb_request_error_t*>(error);
        return std::format("{} value error, code: {}, major: {}, minor: {}", errorNames[er->error_code], er->error_code,
                           er->major_opcode, er->minor_opcode);
    }
    }
    return "unknown error";
}

} // namespace huedra