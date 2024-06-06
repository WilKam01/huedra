#pragma once

#include "graphics/context.hpp"
#include "platform/vulkan/instance.hpp"

namespace huedra {

class VulkanContext : public GraphicalContext
{
public:
    VulkanContext() = default;
    ~VulkanContext() = default;

    void init() override;
    void cleanup() override;

private:
    Instance m_instance;
};

} // namespace huedra