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
    frame += static_cast<u32>(cursorAnimationTimer.passedInterval(Timer::SECONDS_TO_NANO / 30));
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
        log(LogLevel::ERR, "Could not connect wlDisplay!");
    }

    wlConfig.wlRegistry = wl_display_get_registry(wlConfig.wlDisplay);
    if (!wlConfig.wlRegistry)
    {
        log(LogLevel::ERR, "Could not get wlRegistry!");
    }
    wl_registry_add_listener(wlConfig.wlRegistry, &wlConfig.registryListener, &wlConfig);
    wl_display_roundtrip(wlConfig.wlDisplay);

    wlConfig.xkbContext = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (!wlConfig.xkbContext)
    {
        log(LogLevel::ERR, "Could not create xkb context!");
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
    wl_display_dispatch_pending(wlConfig.wlDisplay);
    wl_display_flush(wlConfig.wlDisplay);

    struct pollfd pfd;
    pfd.fd = wl_display_get_fd(wlConfig.wlDisplay);
    pfd.events = POLLIN;

    i32 ret = poll(&pfd, 1, 0);
    if (ret > 0 && (pfd.revents & POLLIN))
    {
        wl_display_dispatch(wlConfig.wlDisplay);
    }

    usleep(1);

    bool resetMousePos{true};
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
            log(LogLevel::WARNING,
                "Wayland: Could not lock mouse, either pointer constraints or pointer object are not available");
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
            log(LogLevel::WARNING,
                "Wayland: Could not confine mouse, either pointer constraints or pointer object are not available");
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
        log(LogLevel::WARNING, "Failed to create window!");
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
        log(LogLevel::WARNING, "Wayland: wp pointer warp or pointer objects not initialized");
        return;
    }

    if (!wlConfig.lastMouseSelectedWindow)
    {
        log(LogLevel::WARNING, "Wayland: not possible to set mouse position with not relative to a window");
        return;
    }

    if (!wlConfig.lastMouseSelectedWindow->isWithinBounds(pos))
    {
        log(LogLevel::WARNING, "Wayland: not possible to set mouse position outside of window bounds");
        return;
    }

    wp_pointer_warp_v1_warp_pointer(wlConfig.wpPointerWarp, wlConfig.lastMouseSelectedWindow->getSurface(),
                                    wlConfig.wlPointer, wl_fixed_from_int(pos.x), wl_fixed_from_int(pos.y),
                                    wlConfig.lastMouseSelectedWindow->getLastPointerSerial());

    wl_display_flush(wlConfig.wlDisplay);
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
#endif
}

void WindowManager::setMouseHidden(bool hidden)
{
#ifdef WIN32
    ShowCursor(static_cast<BOOL>(!hidden));
#endif
}

Window* WindowManager::createWindow(const std::string& title, const WindowInput& input)
{
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
    success = window->init(title, input, wlConfig.wlSharedMemory, wlConfig.wlCompositor, wlConfig.xdgBase,
                           wlConfig.zxdgDecorationManager);
#elif defined(X11)
    // TODO: Implement
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
