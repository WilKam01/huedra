#pragma once

#include "core/types.hpp"

namespace huedra {

class RenderTarget
{
public:
    RenderTarget();
    virtual ~RenderTarget();

    void init(u32 width, u32 height);
    virtual void cleanup() = 0;

    u32 getWidth() { return m_width; }
    u32 getHeight() { return m_height; }
    bool isAvailable() const { return m_available; }

    virtual void prepareNextFrame(u32 frameIndex) = 0;

protected:
    void setAvailability(bool available);

private:
    u32 m_width{0};
    u32 m_height{0};
    bool m_available{true};
};

} // namespace huedra