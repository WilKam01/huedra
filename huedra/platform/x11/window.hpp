#pragma once
#include "window/window.hpp"
#include <xcb/xcb.h>

namespace huedra {

class WindowX11 : public Window
{
public:
    WindowX11() = default;
    ~WindowX11() override = default;

    WindowX11(const WindowX11& rhs) = default;
    WindowX11& operator=(const WindowX11& rhs) = default;
    WindowX11(WindowX11&& rhs) = default;
    WindowX11& operator=(WindowX11&& rhs) = default;

    bool init(const std::string& title, const WindowInput& input, xcb_connection_t* xcbConnection,
              xcb_screen_t* xcbScreen, xcb_intern_atom_reply_t* wmProtoReply, xcb_intern_atom_reply_t* wmDeleteReply);
    void cleanup() override;
    bool update() override;

    bool isWithinBounds(ivec2 position, i32 margin = 0) const override;
    bool isWithinScreenBounds(ivec2 position, i32 margin = 0) const override;
    ivec2 getRelativePosition(ivec2 position) const override;
    ivec2 getRelativeScreenPosition(ivec2 position) const override;

    void setTitle(const std::string& title) override;
    void setResolution(uvec2 resolution) override;
    void setPosition(ivec2 position) override;

    xcb_window_t get() const { return m_window; }
    xcb_connection_t* getConnection() { return m_xcbConnection; }

private:
    std::string getXcbError(xcb_generic_error_t* error);

    xcb_connection_t* m_xcbConnection{nullptr};
    xcb_window_t m_window;
};

} // namespace huedra