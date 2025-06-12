#pragma once

#include "graphics/pipeline_data.hpp"
#include "platform/metal/config.hpp"
#include <Metal/Metal.h>

namespace huedra::converter {

// HU -> MTL
MTLPixelFormat convertPixelDataFormat(GraphicsDataFormat format);

// HU -> MTL
MTLVertexFormat convertDataFormat(GraphicsDataFormat format);

// HU -> MTL
MTLVertexStepFunction convertVertexInputRate(VertexInputRate inputRate);

} // namespace huedra::converter