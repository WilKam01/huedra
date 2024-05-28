#include "window.hpp"

namespace huedra {

void Window::init(const std::string& title, WindowRect rect)
{
    m_title = title;
    m_rect = rect;
}

void Window::cleanup() {}

void Window::updateRect(WindowRect rect) { m_rect = rect; }

} // namespace huedra
