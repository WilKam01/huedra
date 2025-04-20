#include "swapchain.hpp"
#include "graphics/graphics_manager.hpp"

namespace huedra {

void MetalSwapchain::init(id<MTLDevice> device, WindowCocoa* window, bool renderDepth)
{
    m_device = device;
    m_window = window;
    m_renderDepth = renderDepth;

    m_layer = [CAMetalLayer layer];
    m_layer.device = m_device;
    m_layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    m_layer.framebufferOnly = NO;
    m_layer.contentsScale = m_window->getScreenDPI();
    m_layer.frame = m_window->get().contentView.bounds;
    m_layer.maximumDrawableCount = GraphicsManager::MAX_FRAMES_IN_FLIGHT;
    [m_window->get().contentView setLayer:m_layer];
    [m_window->get().contentView setWantsLayer:YES];
}

void MetalSwapchain::cleanup() { [m_layer release]; }

} // namespace huedra