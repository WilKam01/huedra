#include "window.hpp"

namespace huedra {

void Window::init(const std::string& title, Vector2i rect)
{
    m_title = title;
    m_rect = rect;
}

void Window::cleanup() {}

} // namespace huedra
