#pragma once

#include "graphics/context.hpp"
#include "platform/vulkan/device.hpp"
#include "platform/vulkan/instance.hpp"
#include "platform/vulkan/swapchain.hpp"
#include "window/window.hpp"

namespace huedra {

class VulkanContext : public GraphicalContext
{
public:
    VulkanContext() = default;
    ~VulkanContext() = default;

    void init() override;
    void cleanup() override;

    Swapchain* createSwapchain(Window* window) override;
    void removeSwapchain(size_t index) override;

private:
    VkSurfaceKHR createSurface(Window* window);

    Instance m_instance;
    Device m_device;

    std::vector<VulkanSwapchain*> m_swapchains;
    std::vector<VkSurfaceKHR> m_surfaces;
};

} // namespace huedra