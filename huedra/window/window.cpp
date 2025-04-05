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

void Window::updatePosition(i32 xPos, i32 yPos, i32 screenXPos, i32 screenYPos)
{
    m_rect.xPos = xPos;
    m_rect.yPos = yPos;
    m_rect.screenXPos = screenXPos;
    m_rect.screenYPos = screenYPos;
}

void Window::updateResolution(u32 width, u32 height, u32 screenWidth, u32 screenHeight)
{
    m_rect.width = width;
    m_rect.height = height;
    m_rect.screenWidth = screenWidth;
    m_rect.screenHeight = screenHeight;
}

void Window::setRenderTarget(Ref<RenderTarget> renderTarget) { m_renderTarget = std::move(renderTarget); }

} // namespace huedra
