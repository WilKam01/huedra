#pragma once

#include "core/types.hpp"

namespace huedra {

struct WindowInput
{
    u32 width{0};
    u32 height{0};
    std::optional<i32> xPos{};
    std::optional<i32> yPos{};

    WindowInput() = default;
    WindowInput(u32 width, u32 height) : width(width), height(height) {}
    WindowInput(u32 width, u32 height, i32 xPos, i32 yPos) : width(width), height(height), xPos(xPos), yPos(yPos) {}
};

struct WindowRect
{
    u32 width{0};
    u32 height{0};
    u32 screenWidth{0};
    u32 screenHeight{0};
    i32 xPos{0};
    i32 yPos{0};
    i32 xScreenPos{0};
    i32 yScreenPos{0};
};

class Window
{
public:
    Window() = default;
    virtual ~Window() = default;

    void init(const std::string& title, WindowRect rect);
    virtual void cleanup();
    virtual bool update() = 0;

    std::string getTitle() const { return m_title; }
    WindowRect getRect() const { return m_rect; }
    Window* getParent() const { return m_parent; }
    bool shouldClose() const { return m_close; }

    void setParent(Window* parent);
    virtual void setTitle(const std::string& title) = 0;
    virtual void setResolution(u32 width, u32 height) = 0;
    virtual void setPos(i32 x, i32 y) = 0;

protected:
    // Internal use (values updated externally by platform and/or implementation)
    void updateTitle(const std::string& title);
    void updateRect(WindowRect rect);

private:
    std::string m_title{""};
    WindowRect m_rect;
    Window* m_parent{nullptr};
    bool m_close{false};
    std::vector<Window*> m_children;
};
} // namespace huedra