#pragma once
#include "platform/cocoa/window.hpp"
#include "platform/metal/config.hpp"
#include "platform/metal/render_target.hpp"

namespace huedra {

class MetalSwapchain
{
public:
    MetalSwapchain() = default;
    virtual ~MetalSwapchain() = default;

    MetalSwapchain(const MetalSwapchain& rhs) = default;
    MetalSwapchain& operator=(const MetalSwapchain& rhs) = default;
    MetalSwapchain(MetalSwapchain&& rhs) = default;
    MetalSwapchain& operator=(MetalSwapchain&& rhs) = default;

    void init(id<MTLDevice> device, WindowCocoa* window, bool renderDepth);
    void cleanup();

    void aquireNextDrawable();

    CAMetalLayer* getLayer() const { return m_layer; }

private:
    id<MTLDevice> m_device;
    WindowCocoa* m_window;
    CAMetalLayer* m_layer;

    MetalRenderTarget m_renderTarget;
    bool m_renderDepth{true};
};

} // namespace huedra