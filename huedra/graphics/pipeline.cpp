#include "pipeline.hpp"
#include "core/references/reference_counter.hpp"

namespace huedra {

Pipeline::Pipeline() { ReferenceCounter::addResource(static_cast<void*>(this)); }

Pipeline::~Pipeline() { ReferenceCounter::removeResource(static_cast<void*>(this)); }

void Pipeline::init(const PipelineBuilder& pipelineBuilder) { m_builder = pipelineBuilder; }

} // namespace huedra