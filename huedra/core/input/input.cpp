#include "input.hpp"
#include "core/log.hpp"

namespace huedra {

void Input::update() { m_prevKeyDown = m_keyDown; }

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
    return (m_keyDown[index] & bit) && (m_prevKeyDown[index] & bit) == 0;
}

bool Input::isKeyReleased(Keys key) const
{
    if (key == Keys::NONE)
    {
        return false;
    }
    u64 index = static_cast<u64>(static_cast<u64>(key) > 63);
    u64 bit = 1ULL << (static_cast<u64>(key) - index * 64);
    return (m_keyDown[index] & bit) == 0 && (m_prevKeyDown[index] & bit);
}

bool Input::isKeyActive(KeyToggles keyToggle) const
{
    u64 bit = 1ULL << (static_cast<u64>(keyToggle));
    return m_keyToggle & bit;
}

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
    u64 bit = 1u << (static_cast<u64>(keyToggle));
    if (isActive)
    {
        m_keyToggle |= bit;
    }
    else
    {
        m_keyToggle &= ~bit;
    }
}

} // namespace huedra