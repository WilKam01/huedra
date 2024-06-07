#include "context.hpp"

namespace huedra {

void VulkanContext::init()
{
    m_instance.init();
    m_device.init(m_instance);
}

void VulkanContext::cleanup()
{
    m_device.cleanup();
    m_instance.cleanup();
}

} // namespace huedra