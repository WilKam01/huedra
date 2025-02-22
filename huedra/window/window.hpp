#pragma once

#include "core/references/ref.hpp"
#include "core/types.hpp"
#include "graphics/render_target.hpp"

namespace huedra {

struct WindowInput
{
    u32 width{0};
    u32 height{0};
    std::optional<i32> xPos;
    std::optional<i32> yPos;
    bool m_renderDepth{true};

    WindowInput() = default;
    WindowInput(u32 width, u32 height, bool renderDepth = true)
        : width(width), height(height), m_renderDepth(renderDepth)
    {}
    WindowInput(u32 width, u32 height, i32 xPos, i32 yPos, bool renderDepth = true)
        : width(width), height(height), xPos(xPos), yPos(yPos), m_renderDepth(renderDepth)
    {}
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
    friend class VulkanSwapchain;

public:
    Window();
    virtual ~Window();

    Window(const Window& rhs) = default;
    Window& operator=(const Window& rhs) = default;
    Window(Window&& rhs) = default;
    Window& operator=(Window&& rhs) = default;

    void init(const std::string& title, WindowRect rect);
    virtual void cleanup();
    virtual bool update() = 0;

    std::string getTitle() const { return m_title; }
    WindowRect getRect() const { return m_rect; }
    Ref<RenderTarget> getRenderTarget() { return m_renderTarget; }
    Ref<Window> getParent() const { return m_parent; }
    bool shouldClose() const { return m_close; }
    bool isMinimized() const { return m_rect.screenWidth == 0 || m_rect.screenHeight == 0; }

    void setParent(Ref<Window> parent);
    virtual void setTitle(const std::string& title) = 0;
    virtual void setResolution(u32 width, u32 height) = 0;
    virtual void setPos(i32 x, i32 y) = 0;

protected:
    // Internal use (values updated externally by platform and/or implementation)
    void updateTitle(const std::string& title);
    void updateRect(WindowRect rect);

private:
    void setRenderTarget(Ref<RenderTarget> renderTarget);

    std::string m_title;
    WindowRect m_rect;
    bool m_close{false};

    Ref<RenderTarget> m_renderTarget{nullptr};

    Ref<Window> m_parent{nullptr};
    std::vector<Ref<Window>> m_children;
};
} // namespace huedra