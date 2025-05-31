#pragma once

#include "graphics/pipeline_data.hpp"
#include "platform/metal/config.hpp"

namespace huedra::converter {

// HU -> MTL
MTLVertexFormat convertDataFormat(GraphicsDataFormat format);

// HU -> MTL
MTLVertexStepFunction convertVertexInputRate(VertexInputRate inputRate);

} // namespace huedra::converter