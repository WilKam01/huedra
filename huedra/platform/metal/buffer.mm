#include "buffer.hpp"
#include "core/global.hpp"

namespace huedra {

void MetalBuffer::init(id<MTLDevice> device, BufferType type, u64 size, BufferUsageFlags usage, const void* data)
{
    Buffer::init(type, usage, size);
    m_device = device;
    MTLResourceOptions resourceOptions;

    if (type == BufferType::STATIC)
    {
        m_buffers.resize(1);
        resourceOptions = MTLResourceStorageModePrivate;
    }
    else
    {
        m_buffers.resize(GraphicsManager::MAX_FRAMES_IN_FLIGHT);
        resourceOptions = MTLResourceStorageModeShared;
    }

    if (data == nullptr)
    {
        for (auto& buffer : m_buffers)
        {
            buffer = [m_device newBufferWithLength:size options:resourceOptions];
        }
    }
    else
    {
        for (auto& buffer : m_buffers)
        {
            buffer = [m_device newBufferWithBytes:data length:size options:resourceOptions];
        }
    }
}

void MetalBuffer::cleanup() {}

void MetalBuffer::write(u64 size, void* data) {}

void MetalBuffer::read(u64 size, void* data) {}

id<MTLBuffer> MetalBuffer::get()
{
    return m_buffers[getType() == BufferType::STATIC ? 0 : global::graphicsManager.getCurrentFrame()];
}

} // namespace huedra