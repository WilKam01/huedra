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

void MetalBuffer::cleanup()
{
    for (auto& buffer : m_buffers)
    {
        [buffer release];
    }
}

void MetalBuffer::write(void* data, u64 size)
{
    if (getType() == BufferType::STATIC)
    {
        log(LogLevel::WARNING, "MetalBuffer::write(): Could not write to buffer, buffer is static");
        return;
    }

    std::memcpy([m_buffers[global::graphicsManager.getCurrentFrame()] contents], data, size);
}

void MetalBuffer::read(void* data, u64 size)
{
    if (getType() == BufferType::STATIC)
    {
        log(LogLevel::WARNING, "MetalBuffer::read(): Could not read to buffer, buffer is static");
        return;
    }

    std::memcpy(data, [m_buffers[global::graphicsManager.getCurrentFrame()] contents], size);
}

id<MTLBuffer> MetalBuffer::get()
{
    return m_buffers[getType() == BufferType::STATIC ? 0 : global::graphicsManager.getCurrentFrame()];
}

} // namespace huedra