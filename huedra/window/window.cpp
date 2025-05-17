#include "window.hpp"

namespace huedra {
Window::Window() { ReferenceCounter::addResource(static_cast<void*>(this)); }

Window::~Window() { ReferenceCounter::removeResource(static_cast<void*>(this)); }

void Window::init(const std::string& title, WindowRect rect)
{
    m_title = title;
    m_rect = rect;
}

bool Window::isWithinBounds(ivec2 position, i32 margin) const
{
    return position.x >= m_rect.positionX + margin &&
           position.x <= m_rect.positionX + static_cast<i32>(m_rect.width) - margin &&
           position.y >= m_rect.positionY + margin &&
           position.y <= m_rect.positionY + static_cast<i32>(m_rect.height) - margin && !m_minimized;
}

bool Window::isWithinScreenBounds(ivec2 position, i32 margin) const
{
    return position.x >= m_rect.screenPositionX + margin &&
           position.x <= m_rect.screenPositionX + static_cast<i32>(m_rect.screenWidth) - margin &&
           position.y >= m_rect.screenPositionY + margin &&
           position.y <= m_rect.screenPositionY + static_cast<i32>(m_rect.screenHeight) - margin && !m_minimized;
}

ivec2 Window::getRelativePosition(ivec2 position) const { return position - ivec2(m_rect.positionX, m_rect.positionY); }

ivec2 Window::getRelativeScreenPosition(ivec2 position) const
{
    return position - ivec2(m_rect.screenPositionX, m_rect.screenPositionY);
}

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
            window->setShouldClose();
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

void Window::updatePosition(i32 positionX, i32 positionY, i32 screenPositionX, i32 screenPositionY)
{
    m_rect.positionX = positionX;
    m_rect.positionY = positionY;
    m_rect.screenPositionX = screenPositionX;
    m_rect.screenPositionY = screenPositionY;
}

void Window::updateResolution(u32 width, u32 height, u32 screenWidth, u32 screenHeight)
{
    m_rect.width = width;
    m_rect.height = height;
    m_rect.screenWidth = screenWidth;
    m_rect.screenHeight = screenHeight;
}

void Window::updateMinimized(bool isMinimized) { m_minimized = isMinimized; }

void Window::setRenderTarget(Ref<RenderTarget> renderTarget) { m_renderTarget = std::move(renderTarget); }

} // namespace huedra
