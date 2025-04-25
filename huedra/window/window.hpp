#pragma once

#include "core/references/ref.hpp"
#include "core/types.hpp"
#include "graphics/render_target.hpp"
#include "math/vec2.hpp"

namespace huedra {

struct WindowInput
{
    u32 width{0};
    u32 height{0};
    std::optional<i32> positionX;
    std::optional<i32> positionY;
    bool renderDepth{true};

    WindowInput() = default;
    WindowInput(u32 width, u32 height, bool renderDepth = true) : width(width), height(height), renderDepth(renderDepth)
    {}
    WindowInput(u32 width, u32 height, i32 positionX, i32 positionY, bool renderDepth = true)
        : width(width), height(height), positionX(positionX), positionY(positionY), renderDepth(renderDepth)
    {}
};

struct WindowRect
{
    u32 width{0};
    u32 height{0};
    u32 screenWidth{0};
    u32 screenHeight{0};
    i32 positionX{0};
    i32 positionY{0};
    i32 screenPositionX{0};
    i32 screenPositionY{0};
};

class Window
{
#ifdef VULKAN
    friend class VulkanSwapchain;
#elif defined(METAL)
    friend class MetalSwapchain;
#endif
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

    bool isWithinBounds(ivec2 position, i32 margin = 0) const;
    bool isWithinScreenBounds(ivec2 position, i32 margin = 0) const;
    ivec2 getRelativePosition(ivec2 position) const;
    ivec2 getRelativeScreenPosition(ivec2 position) const;

    std::string getTitle() const { return m_title; }
    WindowRect getRect() const { return m_rect; }
    Ref<RenderTarget> getRenderTarget() { return m_renderTarget; }
    Ref<Window> getParent() const { return m_parent; }
    bool shouldClose() const { return m_close; }
    bool isMinimized() const { return m_minimized; }

    ivec2 getPosition() const { return {m_rect.positionX, m_rect.positionY}; }
    ivec2 getScreenPosition() const { return {m_rect.screenPositionX, m_rect.screenPositionY}; }
    uvec2 getSize() const { return {m_rect.width, m_rect.height}; }
    uvec2 getScreenSize() const { return {m_rect.screenWidth, m_rect.screenHeight}; }

    void setParent(Ref<Window> parent);
    virtual void setTitle(const std::string& title) = 0;
    virtual void setResolution(u32 width, u32 height) = 0;
    virtual void setPosition(i32 x, i32 y) = 0;

protected:
    // Internal use (values updated externally by platform and/or implementation)
    void updateTitle(const std::string& title);
    void updatePosition(i32 positionX, i32 positionY, i32 screenPositionX, i32 screenPositionY);
    void updateResolution(u32 width, u32 height, u32 screenWidth, u32 screenHeight);
    void updateMinimized(bool isMinimized);

private:
    void setRenderTarget(Ref<RenderTarget> renderTarget);

    std::string m_title;
    WindowRect m_rect;
    bool m_close{false};
    bool m_minimized{false};

    Ref<RenderTarget> m_renderTarget{nullptr};

    Ref<Window> m_parent{nullptr};
    std::vector<Ref<Window>> m_children;
};
} // namespace huedra