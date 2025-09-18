#include "core/types.hpp"
#include "graphics/render_target.hpp"

namespace huedra {

constexpr bool inline usesColor(RenderTargetType type)
{
    return type == RenderTargetType::COLOR || type == RenderTargetType::COLOR_AND_DEPTH;
}

constexpr bool inline usesDepth(RenderTargetType type)
{
    return type == RenderTargetType::DEPTH || type == RenderTargetType::COLOR_AND_DEPTH;
}

} // namespace huedra
