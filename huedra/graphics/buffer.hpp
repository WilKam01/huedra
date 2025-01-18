#pragma once

#include "core/types.hpp"
#include "graphics/pipeline_data.hpp"

namespace huedra {

class Buffer
{
public:
    Buffer();
    virtual ~Buffer();

    virtual void write(u64 size, void* data) = 0;
    virtual void read(u64 size, void* data) = 0;

    BufferType getType() { return m_type; }
    BufferUsageFlags getBufferUsage() { return m_usage; }
    u64 getSize() { return m_size; }

protected:
    void init(BufferType type, BufferUsageFlags usage, u64 size);

private:
    BufferType m_type{BufferType::STATIC};
    BufferUsageFlags m_usage{HU_BUFFER_USAGE_UNDEFINED};
    u64 m_size{0};
};

} // namespace huedra