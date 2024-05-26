#pragma once

#include "core/types.hpp"

namespace huedra {

struct Vector2i // Temp here before real vector impl
{
    u32 x{0};
    u32 y{0};

    Vector2i() = default;
    Vector2i(u32 x, u32 y) : x(x), y(y) {}
};

class Window
{
public:
    Window() = default;
    virtual ~Window() = default;

    void init(const std::string& title, Vector2i rect);
    virtual void cleanup();
    virtual bool update() = 0;

private:
    std::string m_title{""};
    Vector2i m_rect;
};
} // namespace huedra