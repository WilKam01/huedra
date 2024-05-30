#include "window_manager.hpp"
#include <iostream>

#ifdef WIN32
#include "platform/win32/window.hpp"
#endif

namespace huedra {

void WindowManager::init() {}

bool WindowManager::update()
{
    for (auto it = m_windows.begin(); it != m_windows.end();)
    {
        if (!(*it)->update())
        {
            Window* window = *it;
            delete window;
            it = m_windows.erase(it);
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
