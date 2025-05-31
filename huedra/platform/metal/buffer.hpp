#pragma once

#include "graphics/buffer.hpp"
#include "platform/metal/config.hpp"

namespace huedra {

class MetalBuffer : public Buffer
{
public:
    MetalBuffer() = default;
    ~MetalBuffer() = default;

    MetalBuffer(const MetalBuffer& rhs) = default;
    MetalBuffer& operator=(const MetalBuffer& rhs) = default;
    MetalBuffer(MetalBuffer&& rhs) = default;
    MetalBuffer& operator=(MetalBuffer&& rhs) = default;

    void init(id<MTLDevice> device, BufferType type, u64 size, BufferUsageFlags usage, const void* data = nullptr);
    void cleanup();

    void write(u64 size, void* data) override;
    void read(u64 size, void* data) override;

    id<MTLBuffer> get();

private:
    id<MTLDevice> m_device;
    std::vector<id<MTLBuffer>> m_buffers;
};

} // namespace huedra