#include "config.hpp"
#include "core/global.hpp"
#include "core/log.hpp"
#include <cstring>
#include <linux/input-event-codes.h>
#include <systemd/sd-bus.h>
#include <unistd.h>

namespace huedra {

bool WaylandConfig::loadCursorTheme()
{
    std::string cursorTheme;
    i32 cursorSize = 24; // Use as fallback

    // Method 1, try to locate using org.freedesktop.portal.Desktop
    sd_bus* dBus = nullptr;
    i32 r = sd_bus_open_user(&dBus);

    if (r >= 0)
    {
        sd_bus_error error = SD_BUS_ERROR_NULL;
        sd_bus_message* reply = nullptr;

        i32 rCall = sd_bus_call_method(dBus, "org.freedesktop.portal.Desktop", "/org/freedesktop/portal/desktop",
                                       "org.freedesktop.portal.Settings", "Read", &error, &reply, "ss",
                                       "org.gnome.desktop.interface", "cursor-theme");

        if (rCall > 0)
        {
            const char* innerStr;

            // Try normal variant: v -> s
            i32 readResult = sd_bus_message_read(reply, "v", "s", &innerStr);

            if (readResult < 0)
            {
                readResult = sd_bus_message_read(reply, "v", "v", "s", &innerStr);
            }

            if (readResult >= 0 && innerStr != nullptr)
            {
                cursorTheme = innerStr;

                // Get cursor size as well
                rCall = sd_bus_call_method(dBus, "org.freedesktop.portal.Desktop", "/org/freedesktop/portal/desktop",
                                           "org.freedesktop.portal.Settings", "Read", &error, &reply, "ss",
                                           "org.gnome.desktop.interface", "cursor-size");
                if (rCall >= 0)
                {
                    i32 innerVal = 0;

                    // Try normal variant: v -> i
                    readResult = sd_bus_message_read(reply, "v", "i", &innerVal);

                    // If failed, check if using f64 variant: v -> v -> i
                    if (readResult < 0)
                    {
                        readResult = sd_bus_message_read(reply, "v", "v", "i", &innerVal);
                    }

                    if (readResult >= 0)
                    {
                        cursorSize = innerVal;
                    }
                }
            }
            else
            {
                log::func::warn("D-bus: Could not resolve cursor theme, error code: {}", strerror(-readResult));
            }
        }
        else
        {
            log::func::warn("D-bus: Could not reach cursor-theme on org.freedesktop.portal.Desktop service");
        }

        sd_bus_message_unref(reply);
        sd_bus_error_free(&error);
    }

    // Method 2, try using XCURSOR_THEME env variable
    if (cursorTheme.empty())
    {
        const char* envCursor = std::getenv("XCURSOR_THEME");
        if (envCursor && *envCursor)
        {
            cursorTheme = envCursor;

            // Get cursor size as well
            const char* envSizeStr = std::getenv("XCURSOR_SIZE");
            if (envSizeStr && *envSizeStr)
            {
                try
                {
                    cursorSize = std::stoi(envSizeStr);
                }
                catch (...)
                {}
            }
        }
    }

    if (currentCursorTheme != cursorTheme || currentCursorSize != cursorSize)
    {
        if (wlCursorTheme)
        {
            wl_cursor_theme_destroy(wlCursorTheme);
        }

        wlCursorTheme = wl_cursor_theme_load(cursorTheme.c_str(), cursorSize, wlSharedMemory);
        if (!wlCursorTheme)
        {
            log::func::warn("Could not load system theme, using default instead");
            wlCursorTheme = wl_cursor_theme_load(nullptr, 24, wlSharedMemory);
        }

        currentCursorTheme = cursorTheme;
        currentCursorSize = cursorSize;
    }

    return false;
}

void WaylandConfig::updateCursor(bool override)
{
    cursorAnimTimer.update();
    // If we should pick next frame, but if it's only one frame, then theres no point in animating
    if (cursorAnimTimer.passedInterval(cursorAnimDelay) && cursorNumberOfFrames > 1)
    {
        override = true;
    }

    // Cannot set window mouse is not within one of the windows
    // The same if the current cursor (and hidden) in the window is already set
    if (!lastMouseSelectedWindow || ((currentSelectedCursor == global::input.getCursor() &&
                                      currentCursorHiddenStatus == global::input.isMouseHidden()) &&
                                     !override))
    {
        return;
    }
    else if (currentSelectedCursor != global::input.getCursor() ||
             currentCursorHiddenStatus != global::input.isMouseHidden()) // New cursor or hidden status selected
    {
        cursorAnimTimer.resetInterval();
        cursorAnimDelay = 0;
        cursorFrame = 0;
        cursorNumberOfFrames = 0;
    }
    currentSelectedCursor = global::input.getCursor();
    currentCursorHiddenStatus = global::input.isMouseHidden();

    if (!wlCursorTheme || !wlPointer)
    {
        log::func::warn("Wayland: cursor theme or pointer objects not available");
        return;
    }

    if (global::input.isMouseHidden())
    {
        wl_pointer_set_cursor(wlPointer, lastMouseSelectedWindow->getLastPointerSerial(), NULL, 0, 0);
        return;
    }

    wl_cursor* cursor =
        wl_cursor_theme_get_cursor(wlCursorTheme, cursorTypeNames[static_cast<u32>(global::input.getCursor())].data());
    if (!cursor)
    {
        log::func::error("Wayland: could not find cursor: {}",
                         cursorTypeNames[static_cast<u32>(global::input.getCursor())]);
        return;
    }

    wl_cursor_image* image = cursor->images[cursorFrame];
    cursorNumberOfFrames = cursor->image_count;
    cursorFrame = (cursorFrame + 1) % cursorNumberOfFrames;
    cursorAnimDelay = image->delay * constants::MILLISECONDS_TO_NANO;

    wl_buffer* buffer = wl_cursor_image_get_buffer(image);
    if (!buffer)
    {
        return;
    }

    wl_surface* cursorSurface = lastMouseSelectedWindow->getCursorSurface();
    wl_surface_attach(cursorSurface, buffer, 0, 0);
    wl_surface_damage(cursorSurface, 0, 0, image->width, image->height);
    wl_surface_commit(cursorSurface);

    wl_pointer_set_cursor(wlPointer, lastMouseSelectedWindow->getLastPointerSerial(), cursorSurface, image->hotspot_x,
                          image->hotspot_y);
}

void WaylandConfig::handlePing(void* data, xdg_wm_base* base, u32 serial) { xdg_wm_base_pong(base, serial); }

void WaylandConfig::pointerEnter(void* data, wl_pointer* pointer, u32 serial, wl_surface* surface, wl_fixed_t sx,
                                 wl_fixed_t sy)
{
    auto* config = static_cast<WaylandConfig*>(data);
    auto* window = static_cast<WindowWayland*>(wl_surface_get_user_data(surface));

    window->setIsMouseFocused(true);
    window->setLastPointerSerial(serial);
    config->lastMouseSelectedWindow = window;

    global::input.setMousePos(ivec2(wl_fixed_to_int(sx), wl_fixed_to_int(sy)));

    // Check if theme or size has changed
    config->loadCursorTheme();

    // Force change even if current cursor is the same as input, could have be changed by another application
    config->updateCursor(true);
}

void WaylandConfig::pointerLeave(void* data, wl_pointer* pointer, u32 serial, wl_surface* surface)
{
    auto* window = static_cast<WindowWayland*>(wl_surface_get_user_data(surface));
    window->setIsMouseFocused(false);
}

void WaylandConfig::pointerMotion(void* data, wl_pointer* pointer, u32 time, wl_fixed_t sx, wl_fixed_t sy)
{
    global::input.setMousePos(ivec2(wl_fixed_to_int(sx), wl_fixed_to_int(sy)));
}

void WaylandConfig::pointerButton(void* data, wl_pointer* pointer, u32 serial, u32 time, u32 button, u32 state)
{
    auto* config = static_cast<WaylandConfig*>(data);
    MouseButton mouseButton{MouseButton::NONE};
    switch (button)
    {
    case BTN_LEFT:
        mouseButton = MouseButton::LEFT;
        break;
    case BTN_RIGHT:
        mouseButton = MouseButton::RIGHT;
        break;
    case BTN_MIDDLE:
        mouseButton = MouseButton::MIDDLE;
        break;
    case BTN_SIDE:
    case BTN_FORWARD:
        mouseButton = MouseButton::EXTRA1;
        break;
    case BTN_EXTRA:
    case BTN_BACK:
        mouseButton = MouseButton::EXTRA2;
        break;
    default:
        break;
    }
    global::input.setMouseButton(mouseButton, state == WL_POINTER_BUTTON_STATE_PRESSED);

    if (config->lastMouseButtonClick == button && (time - config->lastMouseButtonClickTime) <= 500 &&
        state == WL_POINTER_BUTTON_STATE_PRESSED)
    {
        global::input.setMouseButtonDoubleClick(mouseButton);
        config->lastMouseButtonClickTime = 0;
        return;
    }

    config->lastMouseButtonClick = button;
    if (state == WL_POINTER_BUTTON_STATE_PRESSED)
    {
        config->lastMouseButtonClickTime = time;
    }
}

void WaylandConfig::pointerAxis(void* data, wl_pointer* pointer, u32 time, u32 axis, wl_fixed_t value) {}

void WaylandConfig::pointerAxis120(void* data, wl_pointer* pointer, u32 axis, i32 value)
{
    if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL)
    {
        global::input.setMouseScrollVertical(value / 120.0);
    }
    else if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL)
    {
        global::input.setMouseScrollHorizontal(value / 120.0);
    }
}

void WaylandConfig::relativePointerHandleMotion(void* data, zwp_relative_pointer_v1* zwp_relative_pointer, u32 utime_hi,
                                                u32 utime_lo, wl_fixed_t dx, wl_fixed_t dy, wl_fixed_t dx_unaccel,
                                                wl_fixed_t dy_unaccel)
{
    global::input.setMouseDelta(ivec2(wl_fixed_to_int(dx_unaccel), wl_fixed_to_int(dy_unaccel)));
}

void WaylandConfig::keyboardKeymap(void* data, wl_keyboard* wl_keyboard, u32 format, i32 fd, u32 size) { close(fd); }

void WaylandConfig::keyboardEnter(void* data, wl_keyboard* wl_keyboard, u32 serial, wl_surface* surface, wl_array* keys)
{
    auto* window = static_cast<WindowWayland*>(wl_surface_get_user_data(surface));
    global::windowManager.m_focusedWindow = window;
}

void WaylandConfig::keyboardLeave(void* data, wl_keyboard* wl_keyboard, u32 serial, wl_surface* surface)
{
    if (global::windowManager.m_focusedWindow == data)
    {
        global::windowManager.m_focusedWindow = nullptr;
    }
}

void WaylandConfig::keyboardKey(void* data, wl_keyboard* wl_keyboard, u32 serial, u32 time, u32 key, u32 state)
{
    auto* config = static_cast<WaylandConfig*>(data);
    xkb_keycode_t xkbKey = key + 8;
    char buf[32];

    i32 len = xkb_state_key_get_utf8(config->xkbState, xkbKey, buf, sizeof(buf));
    Keys selectedKey{Keys::NONE};
    if (len > 0)
    {
        if (state == WL_KEYBOARD_KEY_STATE_PRESSED)
        {
            global::input.setCharacter(buf[0]);
        }
        if (buf[0] >= 'A' && buf[0] <= 'Z' || buf[0] >= 'a' && buf[0] <= 'z')
        {
            selectedKey = static_cast<Keys>(static_cast<u32>(Keys::A) + static_cast<u32>(tolower(buf[0]) - 'a'));
        }
    }

    switch (key)
    {
    case KEY_0:
        selectedKey = Keys::NUM_0;
        break;
    case KEY_1:
        selectedKey = Keys::NUM_1;
        break;
    case KEY_2:
        selectedKey = Keys::NUM_2;
        break;
    case KEY_3:
        selectedKey = Keys::NUM_3;
        break;
    case KEY_4:
        selectedKey = Keys::NUM_4;
        break;
    case KEY_5:
        selectedKey = Keys::NUM_5;
        break;
    case KEY_6:
        selectedKey = Keys::NUM_6;
        break;
    case KEY_7:
        selectedKey = Keys::NUM_7;
        break;
    case KEY_8:
        selectedKey = Keys::NUM_8;
        break;
    case KEY_9:
        selectedKey = Keys::NUM_9;
        break;
    case KEY_LEFT:
        selectedKey = Keys::ARR_LEFT;
        break;
    case KEY_RIGHT:
        selectedKey = Keys::ARR_RIGHT;
        break;
    case KEY_UP:
        selectedKey = Keys::ARR_UP;
        break;
    case KEY_DOWN:
        selectedKey = Keys::ARR_DOWN;
        break;
    case KEY_ESC:
        selectedKey = Keys::ESCAPE;
        break;
    case KEY_LEFTSHIFT:
    case KEY_RIGHTSHIFT:
        selectedKey = Keys::SHIFT;
        break;
    case KEY_LEFTCTRL:
    case KEY_RIGHTCTRL:
        selectedKey = Keys::CTRL;
        break;
    case KEY_LEFTALT:
    case KEY_RIGHTALT:
        selectedKey = Keys::ALT;
        break;
    case KEY_TAB:
        selectedKey = Keys::TAB;
        break;
    case KEY_BACKSPACE:
        selectedKey = Keys::BACKSPACE;
        break;
    case KEY_ENTER:
        selectedKey = Keys::ENTER;
        break;
    case KEY_SPACE:
        selectedKey = Keys::SPACE;
        break;
    case KEY_LEFTMETA:
    case KEY_RIGHTMETA:
        selectedKey = Keys::SUPER;
        break;
    case KEY_CAPSLOCK:
        selectedKey = Keys::CAPS_LOCK;
        break;
    case KEY_NUMLOCK:
        selectedKey = Keys::NUM_LOCK;
        break;
    case KEY_SCROLLLOCK:
        selectedKey = Keys::SCR_LOCK;
        break;
    case KEY_F1:
        selectedKey = Keys::F1;
        break;
    case KEY_F2:
        selectedKey = Keys::F2;
        break;
    case KEY_F3:
        selectedKey = Keys::F3;
        break;
    case KEY_F4:
        selectedKey = Keys::F4;
        break;
    case KEY_F5:
        selectedKey = Keys::F5;
        break;
    case KEY_F6:
        selectedKey = Keys::F6;
        break;
    case KEY_F7:
        selectedKey = Keys::F7;
        break;
    case KEY_F8:
        selectedKey = Keys::F8;
        break;
    case KEY_F9:
        selectedKey = Keys::F9;
        break;
    case KEY_F10:
        selectedKey = Keys::F10;
        break;
    case KEY_F11:
        selectedKey = Keys::F11;
        break;
    case KEY_F12:
        selectedKey = Keys::F12;
        break;
    case KEY_INSERT:
        selectedKey = Keys::INSERT;
        break;
    case KEY_DELETE:
        selectedKey = Keys::DEL;
        break;
    case KEY_HOME:
        selectedKey = Keys::HOME;
        break;
    case KEY_END:
        selectedKey = Keys::END;
        break;
    case KEY_PAGEUP:
        selectedKey = Keys::PAGE_UP;
        break;
    case KEY_PAGEDOWN:
        selectedKey = Keys::PAGE_DOWN;
        break;
    case KEY_KPSLASH:
        selectedKey = Keys::NUMPAD_DIVIDE;
        break;
    case KEY_KPASTERISK:
        selectedKey = Keys::NUMPAD_MULT;
        break;
    case KEY_KPMINUS:
        selectedKey = Keys::NUMPAD_MINUS;
        break;
    case KEY_KPPLUS:
        selectedKey = Keys::NUMPAD_PLUS;
        break;
    case KEY_KPDOT:
        selectedKey = Keys::NUMPAD_DOT;
        break;
    case KEY_KP0:
        selectedKey = Keys::NUMPAD_0;
        break;
    case KEY_KP1:
        selectedKey = Keys::NUMPAD_1;
        break;
    case KEY_KP2:
        selectedKey = Keys::NUMPAD_2;
        break;
    case KEY_KP3:
        selectedKey = Keys::NUMPAD_3;
        break;
    case KEY_KP4:
        selectedKey = Keys::NUMPAD_4;
        break;
    case KEY_KP5:
        selectedKey = Keys::NUMPAD_5;
        break;
    case KEY_KP6:
        selectedKey = Keys::NUMPAD_6;
        break;
    case KEY_KP7:
        selectedKey = Keys::NUMPAD_7;
        break;
    case KEY_KP8:
        selectedKey = Keys::NUMPAD_8;
        break;
    case KEY_KP9:
        selectedKey = Keys::NUMPAD_9;
        break;
    default:
        break;
    }

    global::input.setKey(selectedKey, state == WL_KEYBOARD_KEY_STATE_PRESSED);
}

void WaylandConfig::keyboardModifiers(void* data, wl_keyboard* wl_keyboard, u32 serial, u32 mods_depressed,
                                      u32 mods_latched, u32 mods_locked, u32 group)
{
    auto* config = static_cast<WaylandConfig*>(data);
    if (!config->xkbState)
    {
        return;
    }

    xkb_state_update_mask(config->xkbState, mods_depressed, mods_latched, mods_locked, 0, 0, group);
    xkb_mod_mask_t lockedMask = xkb_state_serialize_mods(config->xkbState, XKB_STATE_MODS_LOCKED);

    if (config->hwCapsBit != XKB_MOD_INVALID)
    {
        global::input.setKeyToggle(KeyToggles::CAPS_LOCK, (lockedMask & (1 << config->hwCapsBit)) != 0);
    }

    if (config->hwNumBit != XKB_MOD_INVALID)
    {
        global::input.setKeyToggle(KeyToggles::NUM_LOCK, (lockedMask & (1 << config->hwNumBit)) != 0);
    }

    if (config->hwScrollBit != XKB_MOD_INVALID)
    {
        global::input.setKeyToggle(KeyToggles::SCR_LOCK, (lockedMask & (1 << config->hwScrollBit)) != 0);
    }
}

void WaylandConfig::handleSeatCapabilities(void* data, wl_seat* seat, u32 capabilities)
{
    auto* config = static_cast<WaylandConfig*>(data);
    if ((capabilities & WL_SEAT_CAPABILITY_POINTER) && !config->wlPointer)
    {
        config->wlPointer = wl_seat_get_pointer(seat);
        wl_pointer_add_listener(config->wlPointer, &pointerListener, data);

        if (!config->zwpRelativePointerManager)
        {
            return;
        }

        config->zwpRelativePointer =
            zwp_relative_pointer_manager_v1_get_relative_pointer(config->zwpRelativePointerManager, config->wlPointer);
        zwp_relative_pointer_v1_add_listener(config->zwpRelativePointer, &relativePointerListener, nullptr);
    }
    else if (!(capabilities & WL_SEAT_CAPABILITY_POINTER) && config->wlPointer)
    {
        wl_pointer_destroy(config->wlPointer);
        config->wlPointer = nullptr;
    }

    if ((capabilities & WL_SEAT_CAPABILITY_KEYBOARD) && !config->wlKeyboard)
    {
        config->wlKeyboard = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(config->wlKeyboard, &keyboardListener, data);
    }
    else if (!(capabilities & WL_SEAT_CAPABILITY_KEYBOARD) && config->wlKeyboard)
    {
        wl_keyboard_destroy(config->wlKeyboard);
        config->wlKeyboard = nullptr;
    }
}

void WaylandConfig::handleRegistry(void* data, wl_registry* registry, u32 name, const char* interface, u32 version)
{
    auto* config = static_cast<WaylandConfig*>(data);
    if (strcmp(interface, wl_compositor_interface.name) == 0)
    {
        config->wlCompositor = static_cast<wl_compositor*>(
            wl_registry_bind(registry, name, &wl_compositor_interface, WAYLAND_COMPOSITOR_BIND_VERSION));
        if (!config->wlCompositor)
        {
            log::func::fatal("Could not bind wayland compositor from handleRegistry call!");
        }
    }
    else if (strcmp(interface, wl_shm_interface.name) == 0)
    {
        config->wlSharedMemory = static_cast<wl_shm*>(
            wl_registry_bind(registry, name, &wl_shm_interface, WAYLAND_SHARED_MEMORY_BIND_VERSION));
        if (!config->wlSharedMemory)
        {
            log::func::fatal("Could not bind wayland shared memory from handleRegistry call!");
        }
    }
    else if (strcmp(interface, xdg_wm_base_interface.name) == 0)
    {
        config->xdgBase =
            static_cast<xdg_wm_base*>(wl_registry_bind(registry, name, &xdg_wm_base_interface, XDG_SHELL_BIND_VERSION));
        if (!config->xdgBase)
        {
            log::func::fatal("Could not bind xdg shell from handleRegistry call!");
        }
        xdg_wm_base_add_listener(config->xdgBase, &pingListener, NULL);
    }
    else if (strcmp(interface, zxdg_decoration_manager_v1_interface.name) == 0)
    {
        config->zxdgDecorationManager = static_cast<zxdg_decoration_manager_v1*>(wl_registry_bind(
            registry, name, &zxdg_decoration_manager_v1_interface, ZXDG_DECORATION_MANAGER_BIND_VERSION));
        if (!config->zxdgDecorationManager)
        {
            log::func::fatal("Could not bind zxdg decoration manager from handleRegistry call!");
        }
    }
    else if (strcmp(interface, wl_seat_interface.name) == 0)
    {
        config->wlSeat =
            static_cast<wl_seat*>(wl_registry_bind(registry, name, &wl_seat_interface, WAYLAND_SEAT_BIND_VERSION));
        if (!config->wlSeat)
        {
            log::func::fatal("Could not bind wayland seat from handleRegistry call!");
        }
        wl_seat_add_listener(config->wlSeat, &seatListener, data);
    }
    else if (strcmp(interface, wp_pointer_warp_v1_interface.name) == 0)
    {
        config->wpPointerWarp = static_cast<wp_pointer_warp_v1*>(
            wl_registry_bind(registry, name, &wp_pointer_warp_v1_interface, WP_POINTER_WARP_BIND_VERSION));
        if (!config->wpPointerWarp)
        {
            log::func::fatal("Could not bind wp pointer warp from handleRegistry call!");
        }
    }
    else if (strcmp(interface, zwp_relative_pointer_manager_v1_interface.name) == 0)
    {
        config->zwpRelativePointerManager = static_cast<zwp_relative_pointer_manager_v1*>(wl_registry_bind(
            registry, name, &zwp_relative_pointer_manager_v1_interface, ZWP_RELATIVE_POINTER_MANAGER_BIND_VERSION));
        if (!config->zwpRelativePointerManager)
        {
            log::func::fatal("Could not bind zwp relative pointer manager from handleRegistry call!");
        }
    }
    else if (strcmp(interface, zwp_pointer_constraints_v1_interface.name) == 0)
    {
        config->zwpPointerConstraints = static_cast<zwp_pointer_constraints_v1*>(wl_registry_bind(
            registry, name, &zwp_pointer_constraints_v1_interface, ZWP_POINTER_CONSTRAINTS_BIND_VERSION));
        if (!config->zwpPointerConstraints)
        {
            log::func::fatal("Could not bind zwp pointer contraints from handleRegistry call!");
        }
    }
}

} // namespace huedra
