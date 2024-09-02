#pragma once

#include "core/input/keys.hpp"
#include "core/types.hpp"

namespace huedra {

class Win32Window;

class Input
{
    friend class Win32Window;

public:
    Input() = default;
    ~Input() = default;

    void update();

    bool isKeyDown(Keys key) const;
    bool isKeyPressed(Keys key) const;
    bool isKeyReleased(Keys key) const;
    bool isKeyToggled(KeyToggles keyToggle) const;

private:
    void setKey(Keys key, bool isDown);
    void setKeyToggle(KeyToggles keyToggle, bool isActive);

    std::array<u64, 2> m_keyDown{0};
    std::array<u64, 2> m_prevKeyDown{0};
    u8 m_keyToggle{0};
};

} // namespace huedra