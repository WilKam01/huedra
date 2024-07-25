#pragma once

#include "core/types.hpp"
#include "graphics/pipeline_builder.hpp"

namespace huedra {

class Pipeline
{
public:
    Pipeline();
    virtual ~Pipeline();

    void init(const PipelineBuilder& pipelineBuilder);
    virtual void cleanup() = 0;

protected:
    PipelineBuilder& getBuilder() { return m_builder; }

private:
    PipelineBuilder m_builder;
};

} // namespace huedra