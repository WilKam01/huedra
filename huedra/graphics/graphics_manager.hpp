#pragma

#include "graphics/context.hpp"
#include "window/window.hpp"

namespace huedra {

class GraphicsManager
{
    friend class WindowManager;

public:
    GraphicsManager() = default;
    ~GraphicsManager() = default;

    void init();
    void cleanup();
    void render();

private:
    void createSwapchain(Window* window);
    void removeSwapchain(size_t index);

    GraphicalContext* m_context;
    std::vector<Swapchain*> m_swapchains;
};

} // namespace huedra
