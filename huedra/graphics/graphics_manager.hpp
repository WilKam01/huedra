#pragma

#include "graphics/context.hpp"

namespace huedra {

class GraphicsManager
{
public:
    GraphicsManager() = default;
    ~GraphicsManager() = default;

    void init();
    void cleanup();

private:
    GraphicalContext* m_context;
};

} // namespace huedra
