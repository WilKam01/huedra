#pragma once

#include "core/types.hpp"
#include "graphics/pipeline_data.hpp"

namespace huedra {

class Buffer
{
public:
    Buffer();
    virtual ~Buffer();

    Buffer(const Buffer& rhs) = default;
    Buffer& operator=(const Buffer& rhs) = default;
    Buffer(Buffer&& rhs) = default;
    Buffer& operator=(Buffer&& rhs) = default;

    virtual void write(u64 size, void* data) = 0;
    virtual void read(u64 size, void* data) = 0;

    BufferType getType() const { return m_type; }
    BufferUsageFlags getBufferUsage() const { return m_usage; }
    u64 getSize() const { return m_size; }

protected:
    void init(BufferType type, BufferUsageFlags usage, u64 size);

private:
    BufferType m_type{BufferType::STATIC};
    BufferUsageFlags m_usage{HU_BUFFER_USAGE_UNDEFINED};
    u64 m_size{0};
};

} // namespace huedra