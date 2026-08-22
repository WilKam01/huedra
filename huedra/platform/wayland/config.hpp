#pragma once
#include "core/input/mouse.hpp"
#include "core/timer.hpp"
#include "platform/wayland/window.hpp"

#include <pointer-constraints.h>
#include <pointer-warp.h>
#include <relative-pointer.h>
#include <wayland-cursor.h>
#include <xkbcommon/xkbcommon.h>

namespace huedra {

struct WaylandConfig
{
    wl_display* wlDisplay{nullptr};
    wl_registry* wlRegistry{nullptr};
    wl_compositor* wlCompositor{nullptr};
    wl_shm* wlSharedMemory{nullptr};
    xdg_wm_base* xdgBase{nullptr};
    zxdg_decoration_manager_v1* zxdgDecorationManager{nullptr};
    wl_seat* wlSeat{nullptr};
    wl_pointer* wlPointer{nullptr};
    wl_keyboard* wlKeyboard{nullptr};
    xkb_context* xkbContext{nullptr};
    xkb_state* xkbState{nullptr};
    wp_pointer_warp_v1* wpPointerWarp{nullptr};
    zwp_relative_pointer_manager_v1* zwpRelativePointerManager{nullptr};
    zwp_relative_pointer_v1* zwpRelativePointer{nullptr};
    zwp_pointer_constraints_v1* zwpPointerConstraints{nullptr};
    zwp_locked_pointer_v1* zwpLockedPointer{nullptr};
    zwp_confined_pointer_v1* zwpConfinedPointer{nullptr};
    wl_cursor_theme* wlCursorTheme{nullptr};

    u32 hwCapsBit{XKB_MOD_INVALID};
    u32 hwNumBit{XKB_MOD_INVALID};
    u32 hwScrollBit{XKB_MOD_INVALID};

    u32 lastMouseButtonClick{0};
    u32 lastMouseButtonClickTime{0};
    bool mouseLocked{false};
    bool mouseConfined{false};

    WindowWayland* lastMouseSelectedWindow{nullptr};

    CursorType currentSelectedCursor{CursorType::DEFAULT};
    bool currentCursorHiddenStatus{false};
    std::string currentCursorTheme;
    u32 currentCursorSize{0};

    Timer cursorAnimTimer;
    u64 cursorAnimDelay{0};
    u32 cursorFrame{0};
    u32 cursorNumberOfFrames{0};

    // Registry bind versions
    static const u32 WAYLAND_COMPOSITOR_BIND_VERSION = 4;
    static const u32 WAYLAND_SHARED_MEMORY_BIND_VERSION = 2;
    static const u32 XDG_SHELL_BIND_VERSION = 1;
    static const u32 ZXDG_DECORATION_MANAGER_BIND_VERSION = 1;
    static const u32 WAYLAND_SEAT_BIND_VERSION = 8;
    static const u32 WP_POINTER_WARP_BIND_VERSION = 1;
    static const u32 ZWP_RELATIVE_POINTER_MANAGER_BIND_VERSION = 1;
    static const u32 ZWP_POINTER_CONSTRAINTS_BIND_VERSION = 1;

    // True: New theme detected
    bool loadCursorTheme();
    void updateCursor(bool override = false);

    static void handlePing(void* data, xdg_wm_base* base, u32 serial);

    static void pointerEnter(void* data, wl_pointer* pointer, u32 serial, wl_surface* surface, wl_fixed_t sx,
                             wl_fixed_t sy);
    static void pointerLeave(void* data, wl_pointer* pointer, u32 serial, wl_surface* surface);
    static void pointerMotion(void* data, wl_pointer* pointer, u32 time, wl_fixed_t sx, wl_fixed_t sy);
    static void pointerButton(void* data, wl_pointer* pointer, u32 serial, u32 time, u32 button, u32 state);
    static void pointerAxis(void* data, wl_pointer* pointer, u32 time, u32 axis, wl_fixed_t value);
    static void pointerAxis120(void* data, wl_pointer* pointer, u32 axis, i32 value);
    static void relativePointerHandleMotion(void* data, struct zwp_relative_pointer_v1* zwp_relative_pointer,
                                            u32 utime_hi, u32 utime_lo, wl_fixed_t dx, wl_fixed_t dy,
                                            wl_fixed_t dx_unaccel, wl_fixed_t dy_unaccel);

    static void keyboardKeymap(void* data, wl_keyboard* wl_keyboard, u32 format, i32 fd, u32 size);
    static void keyboardEnter(void* data, wl_keyboard* wl_keyboard, u32 serial, wl_surface* surface, wl_array* keys);
    static void keyboardLeave(void* data, wl_keyboard* wl_keyboard, u32 serial, wl_surface* surface);
    static void keyboardKey(void* data, wl_keyboard* wl_keyboard, u32 serial, u32 time, u32 key, u32 state);
    static void keyboardModifiers(void* data, wl_keyboard* wl_keyboard, u32 serial, u32 mods_depressed,
                                  u32 mods_latched, u32 mods_locked, u32 group);

    static void handleSeatCapabilities(void* data, wl_seat* seat, u32 capabilities);

    static void handleRegistry(void* data, wl_registry* registry, u32 name, const char* interface, u32 version);

    static constexpr xdg_wm_base_listener pingListener{.ping = handlePing};
    static constexpr wl_registry_listener registryListener{.global = handleRegistry};
    static constexpr wl_pointer_listener pointerListener = {
        .enter = pointerEnter,
        .leave = pointerLeave,
        .motion = pointerMotion,
        .button = pointerButton,
        .axis = pointerAxis,
        .frame = [](void* data, struct wl_pointer* wl_pointer) {},
        .axis_source = [](void* data, struct wl_pointer* wl_pointer, uint32_t axis_source) {},
        .axis_stop = [](void* data, struct wl_pointer* wl_pointer, uint32_t time, uint32_t axis) {},
        .axis_value120 = pointerAxis120};
    static constexpr wl_keyboard_listener keyboardListener = {
        .keymap = keyboardKeymap,
        .enter = keyboardEnter,
        .leave = keyboardLeave,
        .key = keyboardKey,
        .modifiers = keyboardModifiers,
        .repeat_info = [](void* data, struct wl_keyboard* wl_keyboard, int32_t rate, int32_t delay) {}};
    static constexpr zwp_relative_pointer_v1_listener relativePointerListener{.relative_motion =
                                                                                  relativePointerHandleMotion};
    static constexpr wl_seat_listener seatListener = {
        .capabilities = handleSeatCapabilities, .name = [](void* data, struct wl_seat* wl_seat, const char* name) {}};

    static constexpr std::array<std::string_view, 15> cursorTypeNames{
        "default", "text",        "wait",  "progress", "crosshair", "pointer",     "openhand",   "move",
        "help",    "not-allowed", "fleur", "n-resize", "e-resize",  "nwse-resize", "nesw-resize"};
};

} // namespace huedra