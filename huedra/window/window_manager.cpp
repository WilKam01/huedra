#include "window_manager.hpp"
#include "core/global.hpp"
#include "core/log.hpp"

#ifdef WIN32
#include "platform/win32/window.hpp"
#elif defined(COCOA)
#include "platform/cocoa/window.hpp"
#include <Cocoa/Cocoa.h>
#endif

namespace huedra {

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
#elif defined(MACOS)
    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

    NSMenu* menubar = [[NSMenu alloc] init];
    NSMenuItem* appMenuItem = [[NSMenuItem alloc] init];
    [menubar addItem:appMenuItem];
    [NSApp setMainMenu:menubar];

    NSMenu* appMenu = [[NSMenu alloc] init];
    [appMenuItem setSubmenu:appMenu];
    [NSApp activateIgnoringOtherApps:YES];
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
        ClientToScreen(hwnd, reinterpret_cast<POINT*>(&rect.left));
        ClientToScreen(hwnd, reinterpret_cast<POINT*>(&rect.right));
        ClipCursor(&rect);
    }
    else
    {
        ClipCursor(nullptr);
    }
#endif

    if (global::input.getMouseMode() == MouseMode::LOCKED && m_focusedWindow != nullptr)
    {
        WindowRect rect = m_focusedWindow->getRect();
        global::input.setMousePos(ivec2(rect.xPos, rect.yPos) +
                                  ivec2(static_cast<i32>(rect.screenWidth), static_cast<i32>(rect.screenHeight)) / 2);
    }

    for (u32 i = 0; i < m_windows.size();)
    {
        Window* window = m_windows[i];
        if (window->shouldClose() || !window->update())
        {
            m_windows.erase(m_windows.begin() + i);
            global::graphicsManager.removeSwapchain(i);

            window->cleanup();
            delete window;
        }
        else
        {
            ++i;
        }
    }

    return !m_windows.empty();
}

void WindowManager::cleanup()
{
    for (auto& window : m_windows)
    {
        window->cleanup();
        delete window;
    }

#ifdef MACOS
    [NSApp terminate:nil];
#endif
}

Ref<Window> WindowManager::addWindow(const std::string& title, const WindowInput& input, Ref<Window> parent)
{
    Window* window = createWindow(title, input);

    if (window != nullptr)
    {
        m_windows.push_back(window);
        global::graphicsManager.createSwapchain(window, input.renderDepth);
        window->setParent(parent);
    }
    else
    {
        log(LogLevel::WARNING, "Failed to create window!");
    }

    return Ref<Window>(window);
}

void WindowManager::setMousePos(ivec2 pos)
{
#ifdef WIN32
    SetCursorPos(pos.x, pos.y);
#endif
}

void WindowManager::setCursor(CursorType cursor)
{
#ifdef WIN32
    switch (cursor)
    {
    case CursorType::DEFAULT:
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
    case CursorType::HAND:
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
    case CursorType::SIZE_NESW:
        SetCursor(LoadCursor(nullptr, IDC_SIZENESW));
        break;
    case CursorType::SIZE_NWSE:
        SetCursor(LoadCursor(nullptr, IDC_SIZENWSE));
        break;
    case CursorType::SIZE_NS:
        SetCursor(LoadCursor(nullptr, IDC_SIZENS));
        break;
    case CursorType::SIZE_WE:
        SetCursor(LoadCursor(nullptr, IDC_SIZEWE));
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
#endif
    if (success)
    {
        ret = window;
    }
    else
    {
        delete window;
    }

    return ret;
}

// NOLINTEND(readability-convert-member-functions-to-static)

} // namespace huedra
