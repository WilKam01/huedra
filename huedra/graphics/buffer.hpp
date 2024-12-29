#pragma once

#include "core/types.hpp"
#include "graphics/context.hpp"
#include "graphics/pipeline_data.hpp"

namespace huedra {

class Buffer
{
public:
    Buffer();
    virtual ~Buffer();

    void init(BufferType type, BufferUsageFlags usage, u64 size, GraphicalContext* context);

    void read(u64 size, void* data);
    void write(u64 size, void* data);
    void setId(u64 id);

    BufferType getType() { return m_type; }
    BufferUsageFlags getBufferUsage() { return m_usage; }
    u64 getSize() { return m_size; }
    u64 getId() { return m_id; };

private:
    BufferType m_type{BufferType::STATIC};
    BufferUsageFlags m_usage{HU_BUFFER_USAGE_UNDEFINED};
    u64 m_size{0};
    GraphicalContext* p_context{nullptr};
    u64 m_id{0};
};

} // namespace huedra