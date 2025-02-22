#include "window.hpp"

namespace huedra {
Window::Window() { ReferenceCounter::addResource(static_cast<void*>(this)); }

Window::~Window() { ReferenceCounter::removeResource(static_cast<void*>(this)); }

void Window::init(const std::string& title, WindowRect rect)
{
    m_title = title;
    m_rect = rect;
}

// No recursion happens since check is done before calling to ensure it's not calling itselt
// NOLINTNEXTLINE(misc-no-recursion)
void Window::cleanup()
{
    m_close = true;
    if (m_parent.valid())
    {
        for (auto it = m_parent->m_children.begin(); it != m_parent->m_children.end(); it++)
        {
            if (it->get() == this)
            {
                m_parent->m_children.erase(it);
                break;
            }
        }
    }
    for (auto& window : m_children)
    {
        if (window.valid() && window.get() != this)
        {
            window->cleanup();
        }
    }
}

void Window::setParent(Ref<Window> parent)
{
    if (parent.valid())
    {
        m_parent = parent;
        m_parent->m_children.emplace_back(this);
    }
}

void Window::updateTitle(const std::string& title) { m_title = title; }

void Window::updateRect(WindowRect rect) { m_rect = rect; }

void Window::setRenderTarget(Ref<RenderTarget> renderTarget) { m_renderTarget = std::move(renderTarget); }

} // namespace huedra
