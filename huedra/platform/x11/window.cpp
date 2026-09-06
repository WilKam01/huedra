#include "window.hpp"
#include "core/log.hpp"

namespace huedra {

bool WindowX11::init(const std::string& title, const WindowInput& input, xcb_connection_t* xcbConnection,
                     xcb_screen_t* xcbScreen, xcb_intern_atom_reply_t* wmProtoReply,
                     xcb_intern_atom_reply_t* wmDeleteReply)
{
    m_xcbConnection = xcbConnection;
    m_window = xcb_generate_id(m_xcbConnection);

    u32 mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    std::vector<u32> list = {0xff222222, XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE |
                                             XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE |
                                             XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_STRUCTURE_NOTIFY |
                                             XCB_EVENT_MASK_FOCUS_CHANGE | XCB_EVENT_MASK_PROPERTY_CHANGE};
    xcb_create_window(m_xcbConnection, XCB_COPY_FROM_PARENT, m_window, xcbScreen->root, input.positionX.value_or(0),
                      input.positionY.value_or(0), input.width, input.height, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      xcbScreen->root_visual, mask, list.data());

    if (wmProtoReply && wmDeleteReply)
    {
        xcb_change_property(m_xcbConnection, XCB_PROP_MODE_REPLACE, m_window, wmProtoReply->atom, 4, 32, 1,
                            &wmDeleteReply->atom);
    }

    xcb_change_property(m_xcbConnection, XCB_PROP_MODE_REPLACE, m_window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8,
                        title.length(), title.c_str());

    xcb_map_window(m_xcbConnection, m_window);
    xcb_flush(m_xcbConnection);

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
    xcb_change_property(m_xcbConnection, XCB_PROP_MODE_REPLACE, m_window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8,
                        title.length(), title.c_str());
    xcb_flush(m_xcbConnection);
    updateTitle(title);
}

void WindowX11::setResolution(uvec2 resolution)
{
    u16 mask = XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT;
    std::vector<u32> list{resolution.x, resolution.y};
    xcb_configure_window(m_xcbConnection, m_window, mask, list.data());
    xcb_flush(m_xcbConnection);
}

void WindowX11::setPosition(ivec2 position)
{
    u16 mask = XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y;
    std::vector<u32> list{static_cast<u32>(position.x), static_cast<u32>(position.y)};
    xcb_configure_window(m_xcbConnection, m_window, mask, list.data());
    xcb_flush(m_xcbConnection);
}

} // namespace huedra