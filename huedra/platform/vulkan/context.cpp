#include "context.hpp"

namespace huedra {

void VulkanContext::init() { m_instance.init(); }

void VulkanContext::cleanup() { m_instance.cleanup(); }

} // namespace huedra