#pragma once

#include "core/input/keys.hpp"
#include "core/input/mouse.hpp"
#include "core/types.hpp"
#include "math/vec2.hpp"
#include "window/window_manager.hpp"

namespace huedra {

class Input
{
    friend class WindowManager;
#ifdef WIN32
    friend class WindowWin32;
#elif defined(MACOS)
    friend class WindowCocoa;
#endif
public:
    Input() = default;
    ~Input() = default;

    Input(const Input& rhs) = delete;
    Input& operator=(const Input& rhs) = delete;
    Input(Input&& rhs) = delete;
    Input& operator=(Input&& rhs) = delete;

    void update();

    bool isKeyDown(Keys key) const;
    bool isKeyPressed(Keys key) const;
    bool isKeyReleased(Keys key) const;
    bool isKeyActive(KeyToggles keyToggle) const;

    char getCharacter() const;

    bool isMouseButtonDown(MouseButton button) const;
    bool isMouseButtonPressed(MouseButton button) const;
    bool isMouseButtonReleased(MouseButton button) const;
    bool isMouseButtonDoubleClicked(MouseButton button) const;

    ivec2 getMousePosition() const;
    ivec2 getMouseDelta() const;

    vec2 getMouseScroll() const; // x = horizontal, y = vertical
    float getMouseScrollVertical() const;
    float getMouseScrollHorizontal() const;

    MouseMode getMouseMode() const { return m_mouseMode; }
    CursorType getCursor() const { return m_cursor; }
    bool isMouseHidden() const { return m_mouseHidden; }

    void setMousePosition(ivec2 pos);
    void setMouseMode(MouseMode mode);
    void setCursor(CursorType cursor);
    void setMouseHidden(bool hidden);
    void toggleMouseHidden();

private:
    void setKey(Keys key, bool isDown);
    void setKeyToggle(KeyToggles keyToggle, bool isActive);
    void setCharacter(char character);

    void setMouseButton(MouseButton button, bool isDown);
    void setMouseButtonDoubleClick(MouseButton button);
    void setMousePos(ivec2 pos);
    void setMouseDelta(ivec2 pos);
    void setMouseScrollVertical(float vertical);
    void setMouseScrollHorizontal(float horizontal);

    std::array<u64, 2> m_keyDown{0};
    std::array<u64, 2> m_prevKeyDown{0};
    u8 m_keyToggle{0};
    char m_character{0};

    u8 m_mouseButtonDown{0};
    u8 m_prevMouseButtonDown{0};
    u8 m_mouseButtonDoubleClicked{0};

    ivec2 m_mousePos{0};
    ivec2 m_mouseDelta{0};
    vec2 m_mouseScroll{0.0f};

    MouseMode m_mouseMode{MouseMode::NORMAL};
    CursorType m_cursor{CursorType::DEFAULT};
    bool m_mouseHidden{false};
};

} // namespace huedra