#pragma once

#include "core/input/keys.hpp"
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
    void setPosition(i32 x, i32 y) override;

    void updatePositionInternal(i32 xPos, i32 yPos, i32 screenXPos, i32 screenYPos);
    void updateResolutionInternal(u32 width, u32 height, u32 screenWidth, u32 screenHeight);
    void setShouldClose() { m_shouldClose = true; }
    double getScreenDPI() const;

private:
    static Keys convertKey(u16 code, char character);

    struct Impl;
    Impl* m_impl;
    bool m_shouldClose{false};
};

} // namespace huedra