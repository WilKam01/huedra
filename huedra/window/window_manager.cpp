#include "window_manager.hpp"
#include <iostream>

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

    RegisterClass(&wc);
#endif
}

bool WindowManager::update()
{
    for (auto it = m_windows.begin(); it != m_windows.end();)
    {
        if ((*it)->shouldClose() || !(*it)->update())
        {
            Window* window = *it;
            it = m_windows.erase(it);
            window->cleanup();
            delete window;
        }
        else
        {
            ++it;
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

Window* WindowManager::createWindow(const std::string& title, const WindowInput& input)
{
    bool success = false;
    Window* ret = nullptr;
#ifdef WIN32
    Win32Window* window = new Win32Window();
    success = window->init(title, input, GetModuleHandle(NULL));
    if (success)
    {
        m_windows.push_back(window);
        ret = window;
    }
    else
    {
        delete window;
    }
#endif

    if (!success)
    {
        std::cout << "Failed to create window!\n";
    }

    return ret;
}

} // namespace huedra
