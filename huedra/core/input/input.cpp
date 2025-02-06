#include "input.hpp"
#include "core/global.hpp"
#include "core/log.hpp"

namespace huedra {

void Input::update()
{
    m_prevKeyDown = m_keyDown;
    m_prevMouseButtonDown = m_mouseButtonDown;
    m_mouseButtonDoubleClicked = 0;
    m_prevMousePos = m_mousePos;
    m_mouseScroll = ivec2(0);
}

bool Input::isKeyDown(Keys key) const
{
    if (key == Keys::NONE)
    {
        return false;
    }
    u64 index = static_cast<u64>(static_cast<u64>(key) > 63);
    u64 bit = 1ULL << (static_cast<u64>(key) - index * 64);
    return m_keyDown[index] & bit;
}

bool Input::isKeyPressed(Keys key) const
{
    if (key == Keys::NONE)
    {
        return false;
    }
    u64 index = static_cast<u64>(static_cast<u64>(key) > 63);
    u64 bit = 1ULL << (static_cast<u64>(key) - index * 64);
    return (m_keyDown[index] & bit) && !(m_prevKeyDown[index] & bit);
}

bool Input::isKeyReleased(Keys key) const
{
    if (key == Keys::NONE)
    {
        return false;
    }
    u64 index = static_cast<u64>(static_cast<u64>(key) > 63);
    u64 bit = 1ULL << (static_cast<u64>(key) - index * 64);
    return !(m_keyDown[index] & bit) && (m_prevKeyDown[index] & bit);
}

bool Input::isKeyActive(KeyToggles keyToggle) const
{
    u64 bit = 1ULL << (static_cast<u64>(keyToggle));
    return m_keyToggle & bit;
}

bool Input::isMouseButtonDown(MouseButton button) const
{
    if (button == MouseButton::NONE || m_mouseMode == MouseMode::DISABLED)
    {
        return false;
    }
    u8 bit = 1ULL << (static_cast<u8>(button));
    return (m_mouseButtonDown & bit) || (m_mouseButtonDoubleClicked & bit);
}

bool Input::isMouseButtonPressed(MouseButton button) const
{
    if (button == MouseButton::NONE || m_mouseMode == MouseMode::DISABLED)
    {
        return false;
    }
    u8 bit = 1ULL << (static_cast<u8>(button));
    return (m_mouseButtonDown & bit) && !(m_prevMouseButtonDown & bit) || (m_mouseButtonDoubleClicked & bit);
}

bool Input::isMouseButtonReleased(MouseButton button) const
{
    if (button == MouseButton::NONE || m_mouseMode == MouseMode::DISABLED)
    {
        return false;
    }
    u8 bit = 1ULL << (static_cast<u8>(button));
    return !(m_mouseButtonDown & bit) && (m_prevMouseButtonDown & bit) || (m_mouseButtonDoubleClicked & bit);
}

bool Input::isMouseButtonDoubleClicked(MouseButton button) const
{
    if (button == MouseButton::NONE || m_mouseMode == MouseMode::DISABLED)
    {
        return false;
    }
    u8 bit = 1ULL << (static_cast<u8>(button));
    return m_mouseButtonDoubleClicked & bit;
}

ivec2 Input::getAbsoluteMousePos() const
{
    if (m_mouseMode == MouseMode::DISABLED)
    {
        return ivec2();
    }
    return m_mousePos;
}

ivec2 Input::getRelativeMousePos(Ref<Window> window) const
{
    if (m_mouseMode == MouseMode::DISABLED)
    {
        return ivec2();
    }
    if (!window.valid())
    {
        log(LogLevel::WARNING, "getRelativeMousePos(): invalid window");
        return m_mousePos;
    }
    return m_mousePos - ivec2(window->getRect().xScreenPos, window->getRect().yScreenPos);
}

ivec2 Input::getMousePosDelta() const
{
    if (m_mouseMode == MouseMode::DISABLED)
    {
        return ivec2();
    }
    return m_mousePos - m_prevMousePos;
}

ivec2 Input::getRawMousePosDelta() const
{
    if (m_mouseMode == MouseMode::DISABLED)
    {
        return ivec2();
    }
    return m_rawMouseDelta;
}

ivec2 Input::getMouseScroll() const
{
    if (m_mouseMode == MouseMode::DISABLED)
    {
        return ivec2();
    }
    return m_mouseScroll;
}

void Input::setMousePos(ivec2 pos)
{
    if (m_mouseMode == MouseMode::DISABLED)
    {
        return;
    }
    Global::windowManager.setMousePos(pos);
}

void Input::setMouseMode(MouseMode mode) { m_mouseMode = mode; }

void Input::setMouseHidden(bool hidden)
{
    m_mouseHidden = hidden;
    Global::windowManager.setMouseHidden(m_mouseHidden);
}

void Input::toggleMouseHidden() { setMouseHidden(!m_mouseHidden); }

void Input::setKey(Keys key, bool isDown)
{
    if (key == Keys::NONE)
    {
        return;
    }

    u64 index = static_cast<u64>(static_cast<u64>(key) > 63);
    u64 bit = 1ULL << (static_cast<u64>(key) - index * 64);

    if (isDown)
    {
        m_keyDown[index] |= bit;
    }
    else
    {
        m_keyDown[index] &= ~bit;
    }
}

void Input::setKeyToggle(KeyToggles keyToggle, bool isActive)
{
    u8 bit = 1u << (static_cast<u8>(keyToggle));
    if (isActive)
    {
        m_keyToggle |= bit;
    }
    else
    {
        m_keyToggle &= ~bit;
    }
}

void Input::setMouseButton(MouseButton button, bool isDown)
{
    u8 bit = 1u << (static_cast<u8>(button));
    if (isDown)
    {
        m_mouseButtonDown |= bit;
    }
    else
    {
        m_mouseButtonDown &= ~bit;
    }
}

void Input::setMouseButtonDoubleClick(MouseButton button)
{
    u8 bit = 1u << (static_cast<u8>(button));
    m_mouseButtonDoubleClicked |= bit;
}

void Input::setMousePosition(ivec2 pos) { m_mousePos = pos; }

void Input::setRawMouseDelta(ivec2 pos) { m_rawMouseDelta = pos; }

void Input::setMouseScrollVertical(i32 vertical) { m_mouseScroll.y = vertical; }

void Input::setMouseScrollHorizontal(i32 horizontal) { m_mouseScroll.x = horizontal; }

} // namespace huedra