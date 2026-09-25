#include "window_manager.hpp"
#include "core/global.hpp"
#include "core/input/mouse.hpp"
#include "core/log.hpp"

#ifdef WIN32
#include "platform/win32/window.hpp"
#elif defined(COCOA)
#include "core/timer.hpp"
#include "platform/cocoa/window.hpp"
#include <AppKit/AppKit.h>
#elif defined(WAYLAND)
#include "core/file/utils.hpp"
#include "platform/wayland/config.hpp"
#include <sys/poll.h>
#include <unistd.h>
#elif defined(X11)
#include "platform/x11/window.hpp"
#include <X11/keysymdef.h>
#include <linux/input-event-codes.h>
#include <unistd.h>
#include <xkbcommon/xkbcommon-x11.h>
#include <xkbcommon/xkbcommon.h>

// Needed for compilation, since in C++ "explicit" is a keyword and can't be used as a name
#define explicit xcb_explicit_fix
#include <xcb/xkb.h>
#undef explicit

#include <xcb/xcb_cursor.h>
#include <xcb/xfixes.h>
#endif

namespace huedra {

#ifdef COCOA
// "Custom" cursors (not available in cocoa, so load them from file instead)
static constexpr std::vector<NSCursor*> loadCursor(NSString* name)
{
    NSString* path = [@"/System/Library/Frameworks/ApplicationServices.framework/Versions/A/Frameworks/"
                       "HIServices.framework/Versions/A/Resources/cursors/" stringByAppendingPathComponent:name];

    NSString* pdf = [path stringByAppendingPathComponent:@"/cursor.pdf"];
    NSDictionary* info =
        [NSDictionary dictionaryWithContentsOfFile:[path stringByAppendingPathComponent:@"/info.plist"]];
    NSImage* cursorImage = [[NSImage alloc] initWithContentsOfFile:pdf];
    if (cursorImage == nil || cursorImage.isValid == NO || info == nil)
    {
        return {[NSCursor arrowCursor]};
    }

    auto frames = static_cast<u32>([[info valueForKey:@"frames"] integerValue]);
    std::vector<NSCursor*> cursors;
    if (frames > 1)
    {
        cursors.resize(frames);
        for (u32 i = 0; i < frames; ++i)
        {
            CGFloat frameHeight = cursorImage.size.height / static_cast<CGFloat>(frames);
            NSRect rect = NSMakeRect(0, frameHeight * static_cast<CGFloat>(i), cursorImage.size.width, frameHeight);
            NSImage* cropped = [[NSImage alloc] initWithSize:rect.size];
            [cropped lockFocus];
            [cursorImage drawAtPoint:NSMakePoint(-rect.origin.x, -rect.origin.y)
                            fromRect:NSMakeRect(0, 0, cursorImage.size.width, cursorImage.size.height)
                           operation:NSCompositingOperationSourceOver
                            fraction:1.0];
            [cropped unlockFocus];
            cursors[i] = [[NSCursor alloc] initWithImage:cropped
                                                 hotSpot:NSMakePoint([[info valueForKey:@"hotx"] doubleValue],
                                                                     [[info valueForKey:@"hoty"] doubleValue])];
        }
    }
    else
    {
        cursors.push_back([[NSCursor alloc] initWithImage:cursorImage
                                                  hotSpot:NSMakePoint([[info valueForKey:@"hotx"] doubleValue],
                                                                      [[info valueForKey:@"hoty"] doubleValue])]);
    }
    return cursors;
};

static std::vector<NSCursor*> waitCursor;
static std::vector<NSCursor*> helpCursor;
static std::vector<NSCursor*> noEntryCursor;
static std::vector<NSCursor*> movingCursor;
static std::vector<NSCursor*> sizeNSCursor;
static std::vector<NSCursor*> sizeWECursor;
static std::vector<NSCursor*> sizeNESWCursor;
static std::vector<NSCursor*> sizeNWSECursor;
static Timer cursorAnimationTimer;

static NSCursor* getMacCursor(CursorType cursor)
{
    static u32 frame = 0;
    // 30Hz fixed interval
    frame += static_cast<u32>(cursorAnimationTimer.passedInterval(constants::SECONDS_TO_NANO / 30));
    switch (cursor)
    {
    case CursorType::DEFAULT:
        return [NSCursor arrowCursor];
    case CursorType::CARET:
        return [NSCursor IBeamCursor];
    case CursorType::WAIT:
    case CursorType::WAIT_IN_BACKGROUND:
        return waitCursor[waitCursor.size() - (frame % waitCursor.size())];
    case CursorType::CROSSHAIR:
        return [NSCursor crosshairCursor];
    case CursorType::HAND_POINT:
        return [NSCursor pointingHandCursor];
    case CursorType::HAND_OPEN:
        return [NSCursor openHandCursor];
    case CursorType::HAND_GRAB:
        return [NSCursor closedHandCursor];
    case CursorType::HELP:
        return helpCursor[0];
    case CursorType::NO_ENTRY:
        return noEntryCursor[0];
    case CursorType::MOVE:
        return movingCursor[0];
    case CursorType::SIZE_NS:
        return sizeNSCursor[0];
    case CursorType::SIZE_WE:
        return sizeWECursor[0];
    case CursorType::SIZE_NESW:
        return sizeNESWCursor[0];
    case CursorType::SIZE_NWSE:
        return sizeNWSECursor[0];
    }
    return [NSCursor arrowCursor];
}
#elif defined(WAYLAND)
static WaylandConfig wlConfig;
#elif defined(X11)
static xcb_connection_t* xcbConnection{nullptr};
static xcb_intern_atom_reply_t* wmProtoReply{nullptr};
static xcb_intern_atom_reply_t* wmDeleteReply{nullptr};
static xcb_screen_t* xcbScreen{nullptr};
static xkb_context* xkbContext{nullptr};
static xkb_keymap* xkbKeymap{nullptr};
static xkb_state* xkbState{nullptr};
static u32 lastMouseButtonClick{0};
static u32 lastMouseButtonClickTime{0};
static bool firstMotion{true};
static bool mouseConfined{false};
static xcb_cursor_context_t* xcbCursorContext{nullptr};
static constexpr std::array<std::string_view, 15> cursorTypeNames{
    "default", "text",        "wait",  "progress", "crosshair", "pointer",     "openhand",   "move",
    "help",    "not-allowed", "fleur", "n-resize", "e-resize",  "nwse-resize", "nesw-resize"};
#endif

// Multiple functions that could be made static.
// They are not meant to be static and will probably not be in the future
// NOLINTBEGIN(readability-convert-member-functions-to-static)
void WindowManager::init()
{
#ifdef WIN32
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowWin32::windowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = "Window Class";
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;

    RegisterClass(&wc);

    // Enable raw mouse input
    RAWINPUTDEVICE rid;
    rid.usUsagePage = 0x01;        // Generic Desktop Controls
    rid.usUsage = 0x02;            // Mouse
    rid.dwFlags = RIDEV_DEVNOTIFY; // Receive input globally
    rid.hwndTarget = nullptr;
    RegisterRawInputDevices(&rid, 1, sizeof(rid));
#elif defined(COCOA)
    @autoreleasepool
    {
        [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

        NSMenu* menubar = [[NSMenu alloc] init];
        NSMenuItem* appMenuItem = [[NSMenuItem alloc] init];
        [menubar addItem:appMenuItem];
        [NSApp setMainMenu:menubar];

        NSMenu* appMenu = [[NSMenu alloc] init];
        [appMenuItem setSubmenu:appMenu];

        waitCursor = loadCursor(@"busybutclickable");
        helpCursor = loadCursor(@"help");
        noEntryCursor = loadCursor(@"notAllowed");
        movingCursor = loadCursor(@"move");
        sizeNSCursor = loadCursor(@"resizenorthsouth");
        sizeWECursor = loadCursor(@"resizeeastwest");
        sizeNESWCursor = loadCursor(@"resizenortheastsouthwest");
        sizeNWSECursor = loadCursor(@"resizenorthwestsoutheast");
        cursorAnimationTimer.init();
    }
#elif defined(WAYLAND)
    wlConfig.wlDisplay = wl_display_connect(nullptr);
    if (!wlConfig.wlDisplay)
    {
        log::func::fatal("Could not connect wlDisplay!");
    }

    wlConfig.wlRegistry = wl_display_get_registry(wlConfig.wlDisplay);
    if (!wlConfig.wlRegistry)
    {
        log::func::fatal("Could not get wlRegistry!");
    }
    wl_registry_add_listener(wlConfig.wlRegistry, &wlConfig.registryListener, &wlConfig);
    wl_display_roundtrip(wlConfig.wlDisplay);

    wlConfig.xkbContext = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (!wlConfig.xkbContext)
    {
        log::func::fatal("Could not create xkb context!");
    }

    xkb_keymap* keymap = xkb_keymap_new_from_names(wlConfig.xkbContext, nullptr, XKB_KEYMAP_COMPILE_NO_FLAGS);
    if (keymap)
    {
        wlConfig.xkbState = xkb_state_new(keymap);

        wlConfig.hwCapsBit = xkb_keymap_mod_get_index(keymap, XKB_MOD_NAME_CAPS);
        wlConfig.hwNumBit = xkb_keymap_mod_get_index(keymap, XKB_MOD_NAME_NUM);
        wlConfig.hwScrollBit = xkb_keymap_mod_get_index(keymap, XKB_VMOD_NAME_SCROLL);

        xkb_keymap_unref(keymap);
    }

    wlConfig.loadCursorTheme();
#elif defined(X11)
    xcbConnection = xcb_connect(nullptr, nullptr);

    if (xcb_connection_has_error(xcbConnection))
    {
        log::func::fatal("Failed to connect to X server via XCB");
    }

    xcb_intern_atom_cookie_t wmProtoCookie = xcb_intern_atom(xcbConnection, 1, 12, "WM_PROTOCOLS");
    wmProtoReply = xcb_intern_atom_reply(xcbConnection, wmProtoCookie, nullptr);

    xcb_intern_atom_cookie_t wmDeleteCookie = xcb_intern_atom(xcbConnection, 0, 16, "WM_DELETE_WINDOW");
    wmDeleteReply = xcb_intern_atom_reply(xcbConnection, wmDeleteCookie, nullptr);

    const xcb_setup_t* setup = xcb_get_setup(xcbConnection);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
    xcbScreen = iter.data;

    xcb_xkb_use_extension_cookie_t xcbXkbExtensionCookie =
        xcb_xkb_use_extension(xcbConnection, XCB_XKB_MAJOR_VERSION, XCB_XKB_MINOR_VERSION);
    xcb_xkb_use_extension_reply_t* xcbXkbExtensionReply =
        xcb_xkb_use_extension_reply(xcbConnection, xcbXkbExtensionCookie, NULL);

    if (!xcbXkbExtensionReply || !xcbXkbExtensionReply->supported)
    {
        log::func::fatal("X11 server does not support XKB -> X11");
    }
    free(xcbXkbExtensionReply);

    i32 deviceId = xkb_x11_get_core_keyboard_device_id(xcbConnection);
    if (deviceId == -1)
    {
        log::func::fatal("Could not find system primary keyboard layout");
    }

    xkbContext = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    xkbKeymap = xkb_x11_keymap_new_from_device(xkbContext, xcbConnection, deviceId, XKB_KEYMAP_COMPILE_NO_FLAGS);
    xkbState = xkb_x11_state_new_from_device(xkbKeymap, xcbConnection, deviceId);

    xcb_xkb_use_extension(xcbConnection, XCB_XKB_MAJOR_VERSION, XCB_XKB_MINOR_VERSION);
    xcb_xkb_get_state_cookie_t xcbXkbCookie = xcb_xkb_get_state(xcbConnection, XCB_XKB_ID_USE_CORE_KBD);
    xcb_xkb_get_state_reply_t* xcbXkbReply = xcb_xkb_get_state_reply(xcbConnection, xcbXkbCookie, NULL);

    if (xcbXkbReply)
    {
        xkb_state_update_mask(xkbState, xcbXkbReply->baseMods, xcbXkbReply->latchedMods, xcbXkbReply->lockedMods,
                              xcbXkbReply->baseGroup, xcbXkbReply->latchedGroup, xcbXkbReply->lockedGroup);
        free(xcbXkbReply);
    }

    global::input.setKeyToggle(KeyToggles::CAPS_LOCK,
                               xkb_state_mod_name_is_active(xkbState, XKB_MOD_NAME_CAPS, XKB_STATE_MODS_EFFECTIVE) > 0);
    global::input.setKeyToggle(KeyToggles::NUM_LOCK,
                               xkb_state_mod_name_is_active(xkbState, XKB_VMOD_NAME_NUM, XKB_STATE_MODS_EFFECTIVE) > 0);
    global::input.setKeyToggle(KeyToggles::SCR_LOCK, xkb_state_mod_name_is_active(xkbState, XKB_VMOD_NAME_SCROLL,
                                                                                  XKB_STATE_MODS_EFFECTIVE) > 0);

    xcb_xfixes_query_version(xcbConnection, XCB_XFIXES_MAJOR_VERSION, XCB_XFIXES_MINOR_VERSION);

    if (xcb_cursor_context_new(xcbConnection, xcbScreen, &xcbCursorContext) < 0)
    {
        log::func::fatal("Could not create xcb cursor context");
    }
#endif
}

bool WindowManager::update()
{
#ifdef WIN32
    if (global::input.getMouseMode() == MouseMode::CONFINED && m_focusedWindow != nullptr)
    {
        RECT rect;
        HWND hwnd = static_cast<WindowWin32*>(m_focusedWindow)->getHandle();
        GetClientRect(hwnd, &rect);
        ClientToScreen(hwnd, std::bit_cast<POINT*>(&rect.left));
        ClientToScreen(hwnd, std::bit_cast<POINT*>(&rect.right));
        ClipCursor(&rect);
    }
    else
    {
        ClipCursor(nullptr);
    }
#elif defined(COCOA)
    @autoreleasepool
    {
        static bool hasActivatedApp{false};
        if (!hasActivatedApp)
        {
            [NSApp finishLaunching];
            dispatch_async(dispatch_get_main_queue(), ^{
              [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
              [NSApp activateIgnoringOtherApps:YES];
              for (auto& window : m_windows)
              {
                  [static_cast<WindowCocoa*>(window)->get() makeKeyAndOrderFront:nil];
              }
            });
            hasActivatedApp = true;
        }

        static bool isHidden{false};

        bool withinAnyWindow{false};
        for (auto& window : m_windows)
        {
            if (window->isWithinScreenBounds(global::input.getMousePosition()))
            {
                withinAnyWindow = true;
            }

            // This is reached if the mouse is on the window handle,
            // result should be false regardless if it's in another window,
            // but only if this window is in front, most often by being focused
            else if (window->isWithinBounds(global::input.getMousePosition()) && m_focusedWindow == window)
            {
                withinAnyWindow = false;
                break;
            }
        }
        if (global::input.isMouseHidden())
        {
            if (withinAnyWindow && !isHidden)
            {
                [NSCursor hide];
                isHidden = true;
            }
            else if (!withinAnyWindow && isHidden)
            {
                [NSCursor unhide];
                isHidden = false;
            }
        }
        else if (isHidden)
        {
            [NSCursor unhide];
            isHidden = false;
        }

        if (global::input.getMouseMode() == MouseMode::CONFINED && m_focusedWindow != nullptr)
        {
            WindowRect rect = m_focusedWindow->getRect();
            ivec2 position = global::input.getMousePosition();
            position.x = std::min(rect.screenPositionX + static_cast<i32>(rect.screenWidth),
                                  std::max(position.x, rect.screenPositionX));
            position.y = std::min(rect.screenPositionY + static_cast<i32>(rect.screenHeight),
                                  std::max(position.y, rect.screenPositionY));

            // Outside bounds
            if (position != global::input.getMousePosition())
            {
                global::input.setMousePosition(position);
            }
        }

        cursorAnimationTimer.update();
        // No point in changing the cursor if it's hidden
        if (!global::input.isMouseHidden())
        {
            if (withinAnyWindow)
            {
                NSCursor* cursor = getMacCursor(global::input.getCursor());
                if (cursor != [NSCursor currentCursor])
                {
                    [cursor set];
                }
            }
            else if ([NSCursor currentCursor] != [NSCursor arrowCursor])
            {
                [[NSCursor arrowCursor] set];
            }
        }
    }
#elif defined(WAYLAND)
    while (wl_display_prepare_read(wlConfig.wlDisplay) != 0)
    {
        wl_display_dispatch_pending(wlConfig.wlDisplay);
    }

    wl_display_flush(wlConfig.wlDisplay);

    struct pollfd pfd;
    pfd.fd = wl_display_get_fd(wlConfig.wlDisplay);
    pfd.events = POLLIN;

    int ret = poll(&pfd, 1, 0);

    if (ret > 0 && (pfd.revents & POLLIN))
    {
        wl_display_read_events(wlConfig.wlDisplay);
        wl_display_dispatch_pending(wlConfig.wlDisplay);
    }
    else
    {
        wl_display_cancel_read(wlConfig.wlDisplay);
    }

    usleep(1);

    bool resetMousePos{true};
#elif defined(X11)
    if ((global::input.getMouseMode() == MouseMode::LOCKED || global::input.getMouseMode() == MouseMode::CONFINED) &&
        !mouseConfined && m_focusedWindow)
    {
        u16 eventMask = XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE;
        xcb_grab_pointer_cookie_t cookie = xcb_grab_pointer(
            xcbConnection, 0, static_cast<WindowX11*>(m_focusedWindow)->get(), eventMask, XCB_GRAB_MODE_ASYNC,
            XCB_GRAB_MODE_ASYNC, static_cast<WindowX11*>(m_focusedWindow)->get(), XCB_NONE, XCB_CURRENT_TIME);

        xcb_grab_pointer_reply_t* reply = xcb_grab_pointer_reply(xcbConnection, cookie, nullptr);
        if (reply)
        {
            free(reply);
        }
        mouseConfined = true;
    }
    else if ((global::input.getMouseMode() != MouseMode::LOCKED &&
              global::input.getMouseMode() != MouseMode::CONFINED && mouseConfined) ||
             ((global::input.getMouseMode() == MouseMode::LOCKED ||
               global::input.getMouseMode() == MouseMode::CONFINED) &&
              mouseConfined && !m_focusedWindow))
    {
        xcb_ungrab_pointer(xcbConnection, XCB_CURRENT_TIME);
        mouseConfined = false;
    }
    else if (global::input.getMouseMode() == MouseMode::LOCKED && mouseConfined)
    {
        setMousePosition(m_focusedWindow->getPosition() + ivec2(m_focusedWindow->getScreenSize()) / 2);
    }

    xcb_generic_event_t* event;
    while ((event = xcb_poll_for_event(xcbConnection)))
    {
        u8 response = event->response_type & ~0x80;
        switch (response)
        {
        case XCB_CLIENT_MESSAGE: {
            auto* cm = std::bit_cast<xcb_client_message_event_t*>(event);
            if (cm->data.data32[0] == wmDeleteReply->atom)
            {
                for (auto* window : m_windows)
                {
                    auto* windowX11 = static_cast<WindowX11*>(window);
                    if (windowX11->get() == cm->window)
                    {
                        window->setShouldClose();
                        break;
                    }
                }
            }
        }
        break;
        case XCB_CONFIGURE_NOTIFY: {
            auto* cfg = std::bit_cast<xcb_configure_notify_event_t*>(event);
            int16_t localX = 0;
            int16_t localY = 0;

            xcb_translate_coordinates_cookie_t translateCookie =
                xcb_translate_coordinates(xcbConnection, cfg->window, xcbScreen->root, localX, localY);
            xcb_translate_coordinates_reply_t* translateReply =
                xcb_translate_coordinates_reply(xcbConnection, translateCookie, nullptr);

            if (translateReply)
            {
                xcb_intern_atom_cookie_t frameCookie = xcb_intern_atom(xcbConnection, 0, 18, "_NET_FRAME_EXTENTS");
                xcb_intern_atom_reply_t* frameReply = xcb_intern_atom_reply(xcbConnection, frameCookie, nullptr);

                i32 screenPosX = translateReply->dst_x;
                i32 screenPosY = translateReply->dst_y;
                u32 screenWidth = cfg->width;
                u32 screenHeight = cfg->height;
                // Preemptively set values to screen equivelant, incase of no reply
                i32 posX = screenPosX;
                i32 posY = screenPosY;
                u32 width = screenWidth;
                u32 height = screenHeight;

                if (frameReply)
                {
                    xcb_get_property_cookie_t propCookie =
                        xcb_get_property(xcbConnection,
                                         0, // 0 = Do not delete the property after reading
                                         cfg->window,
                                         frameReply->atom,  // The _NET_FRAME_EXTENTS atom
                                         XCB_ATOM_CARDINAL, // Expected data type
                                         0,                 // Offset (start reading at index 0)
                                         4                  // Read 4 values (Left, Right, Top, Bottom)
                        );

                    xcb_get_property_reply_t* propReply = xcb_get_property_reply(xcbConnection, propCookie, nullptr);
                    free(frameReply);

                    if (propReply)
                    {
                        // 4 values * 4 bytes = 16 bytes
                        if (xcb_get_property_value_length(propReply) == 16)
                        {
                            u32* extents = std::bit_cast<u32*>(xcb_get_property_value(propReply));

                            posX = screenPosX - extents[0];
                            posY = screenPosY - extents[2];
                            width = screenWidth + extents[0] + extents[1];
                            height = screenHeight + extents[2] + extents[3];
                        }
                        free(propReply);
                    }
                }

                for (auto* window : m_windows)
                {
                    auto* windowX11 = static_cast<WindowX11*>(window);
                    if (windowX11->get() == cfg->window)
                    {
                        window->updatePosition(posX, posY, screenPosX, screenPosY);
                        window->updateResolution(width, height, screenWidth, screenHeight);
                        break;
                    }
                }

                free(translateReply);
            }
        }
        break;
        case XCB_MOTION_NOTIFY: {
            auto* motion = std::bit_cast<xcb_motion_notify_event_t*>(event);

            if (!firstMotion)
            {
                global::input.setMouseDelta(ivec2(motion->root_x, motion->root_y) - global::input.getMousePosition());
            }
            firstMotion = false;

            global::input.setMousePos(ivec2(motion->root_x, motion->root_y));
        }
        break;
        case XCB_PROPERTY_NOTIFY: {
            auto* prop = std::bit_cast<xcb_property_notify_event_t*>(event);

            xcb_intern_atom_cookie_t wmStateCookie = xcb_intern_atom(xcbConnection, 0, 8, "WM_STATE");
            xcb_intern_atom_reply_t* wmStateReply = xcb_intern_atom_reply(xcbConnection, wmStateCookie, NULL);

            if (wmStateReply && prop->atom == wmStateReply->atom)
            {
                xcb_get_property_cookie_t propCookie =
                    xcb_get_property(xcbConnection, 0, prop->window, wmStateReply->atom, wmStateReply->atom, 0, 2);
                xcb_get_property_reply_t* propReply = xcb_get_property_reply(xcbConnection, propCookie, NULL);

                if (propReply)
                {
                    if (xcb_get_property_value_length(propReply) >= 4)
                    {
                        auto* wmStateData = std::bit_cast<u32*>(xcb_get_property_value(propReply));
                        uint32_t windowState = wmStateData[0];
                        for (auto* window : m_windows)
                        {
                            if (static_cast<WindowX11*>(window)->get() == prop->window)
                            {
                                if (windowState == 1)
                                {
                                    window->updateMinimized(false);
                                }
                                else if (windowState == 3)
                                {
                                    window->updateMinimized(true);
                                }
                            }
                        }
                    }
                    free(propReply);
                }
            }
            free(wmStateReply);
        }
        break;
        case XCB_BUTTON_PRESS:
        case XCB_BUTTON_RELEASE: {
            auto* btn = std::bit_cast<xcb_button_press_event_t*>(event);
            MouseButton button{MouseButton::NONE};
            switch (btn->detail)
            {
            case 1:
                button = MouseButton::LEFT;
                break;
            case 2:
                button = MouseButton::MIDDLE;
                break;
            case 3:
                button = MouseButton::RIGHT;
                break;
            case 9:
                button = MouseButton::EXTRA1;
                break;
            case 8:
                button = MouseButton::EXTRA2;
                break;
            case 4:
                global::input.setMouseScrollVertical(1.0f);
                break;
            case 5:
                global::input.setMouseScrollVertical(-1.0f);
                break;
            case 6:
                global::input.setMouseScrollHorizontal(1.0f);
                break;
            case 7:
                global::input.setMouseScrollHorizontal(-1.0f);
                break;
            default:
                break;
            }

            if (button != MouseButton::NONE)
            {
                global::input.setMouseButton(button, response == XCB_BUTTON_PRESS);

                if (lastMouseButtonClick == btn->detail && (btn->time - lastMouseButtonClickTime) <= 500 &&
                    response == XCB_BUTTON_PRESS)
                {
                    global::input.setMouseButtonDoubleClick(button);
                    lastMouseButtonClickTime = 0;
                }
                else
                {
                    lastMouseButtonClick = btn->detail;
                    if (response == XCB_BUTTON_PRESS)
                    {
                        lastMouseButtonClickTime = btn->time;
                    }
                }
            }
        }
        break;
        case XCB_KEY_PRESS:
        case XCB_KEY_RELEASE: {
            auto* key = std::bit_cast<xcb_key_press_event_t*>(event);
            xkb_keycode_t xkbKey = key->detail;
            char buf[32];

            i32 len = xkb_state_key_get_utf8(xkbState, xkbKey, buf, sizeof(buf));
            Keys selectedKey{Keys::NONE};
            if (len > 0)
            {
                if (response == XCB_KEY_PRESS)
                {
                    global::input.setCharacter(buf[0]);
                }
                if (buf[0] >= 'A' && buf[0] <= 'Z' || buf[0] >= 'a' && buf[0] <= 'z')
                {
                    selectedKey =
                        static_cast<Keys>(static_cast<u32>(Keys::A) + static_cast<u32>(tolower(buf[0]) - 'a'));
                }
            }

            xkb_state_update_key(xkbState, xkbKey, response == XCB_KEY_PRESS ? XKB_KEY_DOWN : XKB_KEY_UP);

            switch (xkbKey - 8)
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

            global::input.setKey(selectedKey, response == XCB_KEY_PRESS);

            global::input.setKeyToggle(
                KeyToggles::CAPS_LOCK,
                xkb_state_mod_name_is_active(xkbState, XKB_MOD_NAME_CAPS, XKB_STATE_MODS_EFFECTIVE) > 0);
            global::input.setKeyToggle(
                KeyToggles::NUM_LOCK,
                xkb_state_mod_name_is_active(xkbState, XKB_VMOD_NAME_NUM, XKB_STATE_MODS_EFFECTIVE) > 0);
            global::input.setKeyToggle(
                KeyToggles::SCR_LOCK,
                xkb_state_mod_name_is_active(xkbState, XKB_VMOD_NAME_SCROLL, XKB_STATE_MODS_EFFECTIVE) > 0);
        }
        break;
        case XCB_FOCUS_IN: {
            auto* focus = std::bit_cast<xcb_focus_in_event_t*>(event);
            for (auto* window : m_windows)
            {
                auto* windowX11 = static_cast<WindowX11*>(window);
                if (windowX11->get() == focus->event)
                {
                    m_focusedWindow = window;
                    break;
                }
            }
        }
        break;
        case XCB_FOCUS_OUT: {
            auto* focus = std::bit_cast<xcb_focus_out_event_t*>(event);
            for (auto* window : m_windows)
            {
                auto* windowX11 = static_cast<WindowX11*>(window);
                if (windowX11->get() == focus->event && m_focusedWindow == window)
                {
                    m_focusedWindow = nullptr;
                    break;
                }
            }
        }
        break;
        default:
            break;
        }
        free(event);
    }
    usleep(1);
#endif

#if defined(WIN32) || defined(COCOA)
    if (global::input.getMouseMode() == MouseMode::LOCKED && m_focusedWindow)
    {
        global::input.setMousePosition(m_focusedWindow->getScreenPosition() +
                                       static_cast<ivec2>(m_focusedWindow->getScreenSize()) / 2);
    }
#elif defined(WAYLAND)
    if (global::input.getMouseMode() == MouseMode::LOCKED && m_focusedWindow && !wlConfig.mouseLocked) // Lock mouse
    {
        if (!wlConfig.zwpPointerConstraints || !wlConfig.wlPointer)
        {
            log::func::error("Could not lock mouse, either pointer constraints or pointer object are not available");
            wlConfig.mouseLocked = true; // Stop spam
        }
        else
        {
            global::input.setMousePosition(m_focusedWindow->getScreenPosition() +
                                           static_cast<ivec2>(m_focusedWindow->getScreenSize()) / 2);
            wlConfig.zwpLockedPointer = zwp_pointer_constraints_v1_lock_pointer(
                wlConfig.zwpPointerConstraints, static_cast<WindowWayland*>(m_focusedWindow)->getSurface(),
                wlConfig.wlPointer, nullptr, ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT);
            wlConfig.mouseLocked = true;
        }
    }
    else if (global::input.getMouseMode() != MouseMode::LOCKED && wlConfig.mouseLocked) // Unlock mouse
    {
        if (wlConfig.zwpLockedPointer)
        {
            zwp_locked_pointer_v1_destroy(wlConfig.zwpLockedPointer);
            wlConfig.zwpLockedPointer = nullptr;
        }
        wlConfig.mouseLocked = false;
    }
    if (global::input.getMouseMode() == MouseMode::CONFINED && m_focusedWindow &&
        !wlConfig.mouseConfined) // Confine mouse
    {
        if (!wlConfig.zwpPointerConstraints || !wlConfig.wlPointer)
        {
            log::func::error("Could not confine mouse, either pointer constraints or pointer object are not available");
            wlConfig.mouseConfined = true; // Stop spam
        }
        else
        {
            wlConfig.zwpConfinedPointer = zwp_pointer_constraints_v1_confine_pointer(
                wlConfig.zwpPointerConstraints, static_cast<WindowWayland*>(m_focusedWindow)->getSurface(),
                wlConfig.wlPointer, nullptr, ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT);
            wlConfig.mouseConfined = true;
        }
    }
    else if (global::input.getMouseMode() != MouseMode::CONFINED && wlConfig.mouseConfined) // Unconfine mouse
    {
        if (wlConfig.zwpConfinedPointer)
        {
            zwp_confined_pointer_v1_destroy(wlConfig.zwpConfinedPointer);
            wlConfig.zwpConfinedPointer = nullptr;
        }
        wlConfig.mouseConfined = false;
    }

    wlConfig.updateCursor();
#endif

    for (u32 i = 0; i < m_windows.size();)
    {
        Window* window = m_windows[i];
        if (window->shouldClose() || !window->update())
        {
            m_windows.erase(m_windows.begin() + i);
            if (global::graphicsManager.isInitialized())
            {
                global::graphicsManager.removeSwapchain(i);
            }

#ifdef WAYLAND
            if (wlConfig.lastMouseSelectedWindow == static_cast<WindowWayland*>(window))
            {
                wlConfig.lastMouseSelectedWindow = nullptr;
            }
#endif

            window->cleanup();
            delete window;
        }
        else
        {
            ++i;
#ifdef WAYLAND
            if (resetMousePos && static_cast<WindowWayland*>(window)->isMouseFocused())
            {
                resetMousePos = false;
            }
#endif
        }
    }

#ifdef WAYLAND
    if (resetMousePos)
    {
        global::input.setMousePos(ivec2(0));
    }
#endif

    return !m_windows.empty();
}

void WindowManager::cleanup()
{
    for (auto& window : m_windows)
    {
        window->cleanup();
        delete window;
    }

#ifdef COCOA
    for (auto& cursor : sizeNWSECursor)
    {
        [cursor release];
    }
    for (auto& cursor : sizeNESWCursor)
    {
        [cursor release];
    }
    for (auto& cursor : sizeWECursor)
    {
        [cursor release];
    }
    for (auto& cursor : sizeNSCursor)
    {
        [cursor release];
    }
    for (auto& cursor : movingCursor)
    {
        [cursor release];
    }
    for (auto& cursor : noEntryCursor)
    {
        [cursor release];
    }
    for (auto& cursor : helpCursor)
    {
        [cursor release];
    }
    for (auto& cursor : waitCursor)
    {
        [cursor release];
    }

    [NSApp terminate:nil];
#elif defined(WAYLAND)
    if (wlConfig.wlCursorTheme)
    {
        wl_cursor_theme_destroy(wlConfig.wlCursorTheme);
    }
    if (wlConfig.xkbState)
    {
        xkb_state_unref(wlConfig.xkbState);
    }
    if (wlConfig.xkbContext)
    {
        xkb_context_unref(wlConfig.xkbContext);
    }
    xdg_wm_base_destroy(wlConfig.xdgBase);
    wl_compositor_destroy(wlConfig.wlCompositor);
    wl_registry_destroy(wlConfig.wlRegistry);
    wl_display_disconnect(wlConfig.wlDisplay);
#elif defined(X11)
    xcb_cursor_context_free(xcbCursorContext);
    xkb_state_unref(xkbState);
    xkb_keymap_unref(xkbKeymap);
    xkb_context_unref(xkbContext);
    free(wmDeleteReply);
    free(wmProtoReply);
    xcb_disconnect(xcbConnection);
#endif
}

Ref<Window> WindowManager::addWindow(const std::string& title, const WindowInput& input, Ref<Window> parent)
{
    Window* window = createWindow(title, input);

    if (window != nullptr)
    {
        m_windows.push_back(window);
        if (global::graphicsManager.isInitialized())
        {
            global::graphicsManager.createSwapchain(window, input.renderDepth);
        }
        window->setParent(parent);
    }
    else
    {
        log::func::error("Failed to create window!");
    }

    return Ref<Window>(window);
}

void WindowManager::setMousePosition(ivec2 pos)
{
#ifdef WIN32
    SetCursorPos(pos.x, pos.y);
#elif defined(COCOA)
    if ([NSApp isActive]) // Prevent from locking mouse when unfocusing application
    {
        CGWarpMouseCursorPosition(CGPoint(static_cast<CGFloat>(pos.x), static_cast<CGFloat>(pos.y)));
    }
#elif defined(WAYLAND)
    if (!wlConfig.wpPointerWarp || !wlConfig.wlPointer)
    {
        log::func::error("Wayland: wp pointer warp or pointer objects not initialized");
        return;
    }

    if (!wlConfig.lastMouseSelectedWindow)
    {
        log::func::warn("Wayland: not possible to set mouse position with not relative to a window");
        return;
    }

    if (!wlConfig.lastMouseSelectedWindow->isWithinBounds(pos))
    {
        log::func::warn("Wayland: not possible to set mouse position outside of window bounds");
        return;
    }

    wp_pointer_warp_v1_warp_pointer(wlConfig.wpPointerWarp, wlConfig.lastMouseSelectedWindow->getSurface(),
                                    wlConfig.wlPointer, wl_fixed_from_int(pos.x), wl_fixed_from_int(pos.y),
                                    wlConfig.lastMouseSelectedWindow->getLastPointerSerial());

    wl_display_flush(wlConfig.wlDisplay);
#elif defined(X11)
    xcb_warp_pointer(xcbConnection, XCB_NONE, xcbScreen->root, 0, 0, 0, 0, pos.x, pos.y);
    firstMotion = true; // To avoid large delta mouse movement
    xcb_flush(xcbConnection);
#endif
}

void WindowManager::setCursor(CursorType cursor)
{
#ifdef WIN32
    switch (cursor)
    {
    case CursorType::DEFAULT:
    case CursorType::HAND_OPEN:
    case CursorType::HAND_GRAB:
        SetCursor(LoadCursor(nullptr, IDC_ARROW));
        break;
    case CursorType::CARET:
        SetCursor(LoadCursor(nullptr, IDC_IBEAM));
        break;
    case CursorType::WAIT:
        SetCursor(LoadCursor(nullptr, IDC_WAIT));
        break;
    case CursorType::WAIT_IN_BACKGROUND:
        SetCursor(LoadCursor(nullptr, IDC_APPSTARTING));
        break;
    case CursorType::CROSSHAIR:
        SetCursor(LoadCursor(nullptr, IDC_CROSS));
        break;
    case CursorType::HAND_POINT:
        SetCursor(LoadCursor(nullptr, IDC_HAND));
        break;
    case CursorType::HELP:
        SetCursor(LoadCursor(nullptr, IDC_HELP));
        break;
    case CursorType::NO_ENTRY:
        SetCursor(LoadCursor(nullptr, IDC_NO));
        break;
    case CursorType::MOVE:
        SetCursor(LoadCursor(nullptr, IDC_SIZEALL));
        break;
    case CursorType::SIZE_NS:
        SetCursor(LoadCursor(nullptr, IDC_SIZENS));
        break;
    case CursorType::SIZE_WE:
        SetCursor(LoadCursor(nullptr, IDC_SIZEWE));
        break;
    case CursorType::SIZE_NESW:
        SetCursor(LoadCursor(nullptr, IDC_SIZENESW));
        break;
    case CursorType::SIZE_NWSE:
        SetCursor(LoadCursor(nullptr, IDC_SIZENWSE));
        break;
    }
#elif defined(X11)
    xcb_cursor_t xcbCursor = xcb_cursor_load_cursor(xcbCursorContext, cursorTypeNames[static_cast<u32>(cursor)].data());
    if (xcbCursor != XCB_NONE)
    {
        for (auto* window : m_windows)
        {
            xcb_change_window_attributes(xcbConnection, static_cast<WindowX11*>(window)->get(), XCB_CW_CURSOR,
                                         &xcbCursor);
        }
        xcb_flush(xcbConnection);
    }
    else
    {
        log::func::error("X11, Could not find cursor: {}", cursorTypeNames[static_cast<u32>(cursor)]);
    }
#endif
}

void WindowManager::setMouseHidden(bool hidden)
{
#ifdef WIN32
    ShowCursor(static_cast<BOOL>(!hidden));
#elif defined(X11)
    if (hidden)
    {
        for (auto* window : m_windows)
        {
            xcb_xfixes_hide_cursor(xcbConnection, static_cast<WindowX11*>(window)->get());
        }
    }
    else
    {
        for (auto* window : m_windows)
        {
            xcb_xfixes_show_cursor(xcbConnection, static_cast<WindowX11*>(window)->get());
        }
    }
    xcb_flush(xcbConnection);
#endif
}

Window* WindowManager::createWindow(const std::string& title, const WindowInput& input)
{
    if (input.width == 0 || input.height == 0)
    {
        log::func::error("Could not create window, resolution: ({}, {}) not allowed", input.width, input.height);
        return nullptr;
    }

    Window* ret = nullptr;
    bool success = false;
#ifdef WIN32
    auto* window = new WindowWin32();
    success = window->init(title, input, GetModuleHandle(nullptr));
#elif defined(COCOA)
    auto* window = new WindowCocoa();
    success = window->init(title, input);
#elif defined(WAYLAND)
    auto* window = new WindowWayland();
    success =
        window->init(title, input, global::graphicsManager.isInitialized(), wlConfig.wlDisplay, wlConfig.wlSharedMemory,
                     wlConfig.wlCompositor, wlConfig.xdgBase, wlConfig.zxdgDecorationManager);
#elif defined(X11)
    auto* window = new WindowX11();
    success = window->init(title, input, xcbConnection, xcbScreen, wmProtoReply, wmDeleteReply);
#endif
    if (success)
    {
        ret = window;
    }
    else
    {
        delete window;
    }

#ifdef WAYLAND
    wl_display_roundtrip(wlConfig.wlDisplay);
#endif

    return ret;
}

// NOLINTEND(readability-convert-member-functions-to-static)

} // namespace huedra
