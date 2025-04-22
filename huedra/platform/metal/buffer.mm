#include "buffer.hpp"

namespace huedra {

void MetalBuffer::init(id<MTLDevice> device, BufferType type, u64 size, BufferUsageFlags usage, const void* data)
{
    Buffer::init(type, usage, size);
    m_device = device;
}

void MetalBuffer::cleanup() {}

void MetalBuffer::write(u64 size, void* data) {}

void MetalBuffer::read(u64 size, void* data) {}

} // namespace huedra