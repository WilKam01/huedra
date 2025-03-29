#pragma once

#include "window/window.hpp"

namespace huedra {

class WindowCocoa : public Window
{
public:
    WindowCocoa() = default;
    ~WindowCocoa() = default;

    WindowCocoa(const WindowCocoa& rhs) = default;
    WindowCocoa& operator=(const WindowCocoa& rhs) = default;
    WindowCocoa(WindowCocoa&& rhs) = default;
    WindowCocoa& operator=(WindowCocoa&& rhs) = default;

    bool init(const std::string& title, const WindowInput& input);
    void cleanup() override;
    bool update() override;

    void setTitle(const std::string& title) override;
    void setResolution(u32 width, u32 height) override;
    void setPos(i32 x, i32 y) override;

private:
};

} // namespace huedra