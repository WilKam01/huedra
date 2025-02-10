#include "window_manager.hpp"
#include "core/global.hpp"
#include "core/log.hpp"

#ifdef WIN32
#include "platform/win32/window.hpp"
#endif

namespace huedra {

void WindowManager::init()
{
#ifdef WIN32
    const char CLASS_NAME[] = "Window Class";

    WNDCLASS wc = {};
    wc.lpfnWndProc = Win32Window::WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;

    RegisterClass(&wc);

    // Enable raw mouse input
    RAWINPUTDEVICE rid;
    rid.usUsagePage = 0x01;        // Generic Desktop Controls
    rid.usUsage = 0x02;            // Mouse
    rid.dwFlags = RIDEV_DEVNOTIFY; // Receive input globally
    rid.hwndTarget = NULL;
    RegisterRawInputDevices(&rid, 1, sizeof(rid));
#endif
}

bool WindowManager::update()
{
#ifdef WIN32
    if (Global::input.getMouseMode() == MouseMode::CONFINED && p_focusedWindow)
    {
        RECT rect;
        HWND hwnd = static_cast<Win32Window*>(p_focusedWindow)->getHandle();
        GetClientRect(hwnd, &rect);
        ClientToScreen(hwnd, (POINT*)&rect.left);
        ClientToScreen(hwnd, (POINT*)&rect.right);
        ClipCursor(&rect);
    }
    else
    {
        ClipCursor(NULL);
    }
#endif

    if (Global::input.getMouseMode() == MouseMode::LOCKED && p_focusedWindow)
    {
        WindowRect rect = p_focusedWindow->getRect();
        Global::input.setMousePos(ivec2(rect.xPos, rect.yPos) + ivec2(rect.screenWidth, rect.screenHeight) / 2);
    }

    for (u32 i = 0; i < m_windows.size();)
    {
        Window* window = m_windows[i];
        if (window->shouldClose() || !window->update())
        {
            m_windows.erase(m_windows.begin() + i);
            Global::graphicsManager.removeSwapchain(i);

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
}

Ref<Window> WindowManager::addWindow(const std::string& title, const WindowInput& input, Ref<Window> parent)
{
    Window* window = createWindow(title, input);

    if (window)
    {
        m_windows.push_back(window);
        Global::graphicsManager.createSwapchain(window, input.m_renderDepth);
        if (parent.valid())
        {
            window->setParent(parent);
        }
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
        SetCursor(LoadCursor(NULL, IDC_ARROW));
        break;
    case CursorType::CARET:
        SetCursor(LoadCursor(NULL, IDC_IBEAM));
        break;
    case CursorType::WAIT:
        SetCursor(LoadCursor(NULL, IDC_WAIT));
        break;
    case CursorType::WAIT_IN_BACKGROUND:
        SetCursor(LoadCursor(NULL, IDC_APPSTARTING));
        break;
    case CursorType::CROSSHAIR:
        SetCursor(LoadCursor(NULL, IDC_CROSS));
        break;
    case CursorType::HAND:
        SetCursor(LoadCursor(NULL, IDC_HAND));
        break;
    case CursorType::HELP:
        SetCursor(LoadCursor(NULL, IDC_HELP));
        break;
    case CursorType::NO_ENTRY:
        SetCursor(LoadCursor(NULL, IDC_NO));
        break;
    case CursorType::MOVE:
        SetCursor(LoadCursor(NULL, IDC_SIZEALL));
        break;
    case CursorType::SIZE_NESW:
        SetCursor(LoadCursor(NULL, IDC_SIZENESW));
        break;
    case CursorType::SIZE_NWSE:
        SetCursor(LoadCursor(NULL, IDC_SIZENWSE));
        break;
    case CursorType::SIZE_NS:
        SetCursor(LoadCursor(NULL, IDC_SIZENS));
        break;
    case CursorType::SIZE_WE:
        SetCursor(LoadCursor(NULL, IDC_SIZEWE));
        break;
    }
#endif
}

void WindowManager::setMouseHidden(bool hidden)
{
#ifdef WIN32
    ShowCursor(!hidden);
#endif
}

Window* WindowManager::createWindow(const std::string& title, const WindowInput& input)
{
    Window* ret = nullptr;
    bool success = false;
#ifdef WIN32
    Win32Window* window = new Win32Window();
    success = window->init(title, input, GetModuleHandle(NULL));
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

} // namespace huedra
