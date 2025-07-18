#include "input.hpp"
#include "core/global.hpp"
#include "core/log.hpp"
#include "math/vec_transform.hpp"

namespace huedra {

void Input::update()
{
    m_prevKeyDown = m_keyDown;
    m_prevMouseButtonDown = m_mouseButtonDown;
    m_mouseButtonDoubleClicked = 0;
    m_mouseDelta = ivec2(0);
    m_mouseScroll = vec2(0.0f);
    m_character = 0;
}

bool Input::isKeyDown(Keys key) const
{
    if (key == Keys::NONE)
    {
        return false;
    }
    u64 index = static_cast<u64>(static_cast<u8>(key) > 63);
    u64 bit = 1ULL << (static_cast<u64>(key) - index * 64);
    return (m_keyDown[index] & bit) != 0u;
}

bool Input::isKeyPressed(Keys key) const
{
    if (key == Keys::NONE)
    {
        return false;
    }
    u64 index = static_cast<u64>(static_cast<u64>(key) > 63);
    u64 bit = 1ULL << (static_cast<u64>(key) - index * 64);
    return (m_keyDown[index] & bit) != 0u && (m_prevKeyDown[index] & bit) == 0u;
}

bool Input::isKeyReleased(Keys key) const
{
    if (key == Keys::NONE)
    {
        return false;
    }
    u64 index = static_cast<u64>(static_cast<u64>(key) > 63);
    u64 bit = 1ULL << (static_cast<u64>(key) - index * 64);
    return (m_keyDown[index] & bit) == 0u && (m_prevKeyDown[index] & bit) != 0u;
}

bool Input::isKeyActive(KeyToggles keyToggle) const
{
    u64 bit = 1ULL << (static_cast<u64>(keyToggle));
    return (m_keyToggle & bit) != 0u;
}

char Input::getCharacter() const { return m_character; }

bool Input::isMouseButtonDown(MouseButton button) const
{
    if (button == MouseButton::NONE || m_mouseMode == MouseMode::DISABLED)
    {
        return false;
    }
    u8 bit = 1ULL << (static_cast<u8>(button));
    return (m_mouseButtonDown & bit) != 0u || (m_mouseButtonDoubleClicked & bit) != 0u;
}

bool Input::isMouseButtonPressed(MouseButton button) const
{
    if (button == MouseButton::NONE || m_mouseMode == MouseMode::DISABLED)
    {
        return false;
    }
    u8 bit = 1ULL << (static_cast<u8>(button));
    return (m_mouseButtonDown & bit) != 0u && (m_prevMouseButtonDown & bit) == 0u ||
           (m_mouseButtonDoubleClicked & bit) != 0u;
}

bool Input::isMouseButtonReleased(MouseButton button) const
{
    if (button == MouseButton::NONE || m_mouseMode == MouseMode::DISABLED)
    {
        return false;
    }
    u8 bit = 1ULL << (static_cast<u8>(button));
    return (m_mouseButtonDown & bit) == 0u && (m_prevMouseButtonDown & bit) != 0u ||
           (m_mouseButtonDoubleClicked & bit) != 0u;
}

bool Input::isMouseButtonDoubleClicked(MouseButton button) const
{
    if (button == MouseButton::NONE || m_mouseMode == MouseMode::DISABLED)
    {
        return false;
    }
    u8 bit = 1ULL << (static_cast<u8>(button));
    return (m_mouseButtonDoubleClicked & bit) != 0u;
}

ivec2 Input::getMousePosition() const
{
    if (m_mouseMode == MouseMode::DISABLED)
    {
        return {};
    }
    return m_mousePos;
}

ivec2 Input::getMouseDelta() const
{
    if (m_mouseMode == MouseMode::DISABLED)
    {
        return {};
    }
    return m_mouseDelta;
}

vec2 Input::getMouseScroll() const
{
    if (m_mouseMode == MouseMode::DISABLED)
    {
        return {};
    }
    return m_mouseScroll;
}

float Input::getMouseScrollVertical() const
{
    if (m_mouseMode == MouseMode::DISABLED)
    {
        return 0.0f;
    }
    return m_mouseScroll.y;
}

float Input::getMouseScrollHorizontal() const
{
    if (m_mouseMode == MouseMode::DISABLED)
    {
        return 0.0f;
    }
    return m_mouseScroll.x;
}

void Input::setMousePosition(ivec2 pos)
{
    if (m_mouseMode == MouseMode::DISABLED)
    {
        return;
    }
    m_mousePos = pos;
    global::windowManager.setMousePosition(pos);
}

void Input::setMouseMode(MouseMode mode)
{
    if (m_mouseMode == MouseMode::DISABLED && mode != MouseMode::DISABLED)
    {
        global::windowManager.setCursor(m_cursor);
        global::windowManager.setMouseHidden(m_mouseHidden);
    }
    m_mouseMode = mode;
}

void Input::setCursor(CursorType cursor)
{
    m_cursor = cursor;
    if (m_mouseMode != MouseMode::DISABLED)
    {
        global::windowManager.setCursor(m_cursor);
    }
}

void Input::setMouseHidden(bool hidden)
{
    m_mouseHidden = hidden;
    if (m_mouseMode != MouseMode::DISABLED)
    {
        global::windowManager.setMouseHidden(m_mouseHidden);
    }
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

void Input::setCharacter(char character) { m_character = character; }

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

void Input::setMousePos(ivec2 pos) { m_mousePos = pos; }

void Input::setMouseDelta(ivec2 pos) { m_mouseDelta = pos; }

void Input::setMouseScrollVertical(float vertical) { m_mouseScroll.y = vertical; }

void Input::setMouseScrollHorizontal(float horizontal) { m_mouseScroll.x = horizontal; }

} // namespace huedra