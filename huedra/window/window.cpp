#include "window.hpp"

namespace huedra {

void Window::init(const std::string& title, WindowRect rect)
{
    m_title = title;
    m_rect = rect;
}

void Window::cleanup()
{
    m_close = true;
    if (m_parent)
    {
        for (auto it = m_parent->m_children.begin(); it != m_parent->m_children.end(); it++)
        {
            if ((*it) == this)
            {
                m_parent->m_children.erase(it);
                break;
            }
        }
    }
    for (auto& window : m_children)
    {
        window->cleanup();
    }
}

void Window::setParent(Window* parent)
{
    if (parent)
    {
        m_parent = parent;
        m_parent->m_children.push_back(this);
    }
}

void Window::updateTitle(const std::string& title) { m_title = title; }

void Window::updateRect(WindowRect rect) { m_rect = rect; }

} // namespace huedra
