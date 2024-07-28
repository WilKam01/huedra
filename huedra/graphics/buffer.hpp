#pragma once

#include "core/types.hpp"
#include "graphics/pipeline_data.hpp"

namespace huedra {

class Buffer
{
public:
    Buffer();
    virtual ~Buffer();

    void init(BufferType type, BufferUsageFlags usage, u64 size);
    virtual void cleanup() = 0;

    virtual void write(void* data, u64 size) = 0;
    virtual void read(void* data, u64 size) = 0;

    BufferType getType() { return m_type; }
    BufferUsageFlags getBufferUsage() { return m_usage; }
    u64 getSize() { return m_size; }

private:
    BufferType m_type{BufferType::STATIC};
    BufferUsageFlags m_usage{HU_BUFFER_USAGE_UNDEFINED};
    u64 m_size{0};
};

} // namespace huedra