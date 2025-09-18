#include "render_pass_builder.hpp"
#include "core/log.hpp"
#include "graphics/pipeline_data.hpp"
#include "graphics/render_target.hpp"

namespace huedra {

RenderPassBuilder& RenderPassBuilder::init(RenderPassType type, RenderTargetType renderTargetUse)
{
    m_type = type;
    m_renderTargetUse = renderTargetUse;
    m_commands = nullptr;
    m_inputs.clear();
    m_outputs.clear();
    m_renderTargets.clear();

    return *this;
}

RenderPassBuilder& RenderPassBuilder::setPipeline(const PipelineBuilder& pipeline)
{
    if (pipeline.empty())
    {
        log(LogLevel::WARNING, "RenderPassBuilder: pipeline invalid, is empty");
        return *this;
    }

    if (m_type == RenderPassType::GRAPHICS && pipeline.getType() != PipelineType::GRAPHICS)
    {
        log(LogLevel::WARNING, "RenderPassBuilder: pipeline invalid, not a graphics pipeline");
        return *this;
    }

    if (m_type == RenderPassType::COMPUTE && pipeline.getType() != PipelineType::COMPUTE)
    {
        log(LogLevel::WARNING, "RenderPassBuilder: pipeline invalid, not a compute pipeline");
        return *this;
    }

    if (m_type == RenderPassType::GRAPHICS && !pipeline.getShaderStages().contains(ShaderStage::VERTEX))
    {
        log(LogLevel::WARNING, "RenderPassBuilder: graphics pipeline invalid, no vertex shader present");
        return *this;
    }

    m_pipeline = pipeline;
    return *this;
}

RenderPassBuilder& RenderPassBuilder::setCommands(const RenderCommands& commands)
{
    m_commands = commands;
    return *this;
}

RenderPassBuilder& RenderPassBuilder::setClearRenderTargets(bool clearRenderTargets, RenderTargetType renderTargetUse)
{
    m_clearTargets = clearRenderTargets;
    m_renderTargetClearUse = renderTargetUse;
    return *this;
}

RenderPassBuilder& RenderPassBuilder::addResource(ResourceAccessType access, Ref<Buffer> buffer,
                                                  ShaderStage shaderStage)
{
    if (!buffer.valid())
    {
        log(LogLevel::WARNING, "RenderPassBuilder: buffer invalid");
        return *this;
    }

    RenderPassReference reference;
    reference.access = access;
    reference.shaderStage = shaderStage;
    reference.buffer = buffer.get();
    reference.type = RenderPassReference::Type::BUFFER;

    if (access == ResourceAccessType::READ || access == ResourceAccessType::READ_WRITE)
    {
        m_inputs.push_back(reference);
    }

    if (access == ResourceAccessType::WRITE || access == ResourceAccessType::READ_WRITE)
    {
        m_outputs.push_back(reference);
    }

    return *this;
}

RenderPassBuilder& RenderPassBuilder::addResource(ResourceAccessType access, Ref<Texture> texture,
                                                  ShaderStage shaderStage)
{
    if (!texture.valid())
    {
        log(LogLevel::WARNING, "RenderPassBuilder: texture invalid");
        return *this;
    }

    RenderPassReference reference;
    reference.access = access;
    reference.shaderStage = shaderStage;
    reference.texture = texture.get();
    reference.type = RenderPassReference::Type::TEXTURE;

    if (access == ResourceAccessType::READ || access == ResourceAccessType::READ_WRITE)
    {
        m_inputs.push_back(reference);
    }

    if (access == ResourceAccessType::WRITE || access == ResourceAccessType::READ_WRITE)
    {
        m_outputs.push_back(reference);
    }

    return *this;
}

RenderPassBuilder& RenderPassBuilder::addRenderTarget(Ref<RenderTarget> renderTarget, vec3 clearColor)
{
    if (!renderTarget.valid() || !renderTarget->isAvailable())
    {
        log(LogLevel::WARNING, "RenderPassBuilder: render target not available");
        return *this;
    }

    if (m_type != RenderPassType::GRAPHICS)
    {
        log(LogLevel::WARNING, "RenderPassBuilder: could not add render target to non graphics render pass");
        return *this;
    }

    // May not be valid depending on render target use
    if (renderTarget->getType() != RenderTargetType::COLOR_AND_DEPTH)
    {
        if (m_renderTargetUse == RenderTargetType::COLOR && !renderTarget->usesColor() ||
            m_renderTargetUse == RenderTargetType::DEPTH && !renderTarget->usesDepth() ||
            m_renderTargetUse == RenderTargetType::COLOR_AND_DEPTH)
        {
            log(LogLevel::WARNING,
                R"(RenderPassBuilder: could not add render target to render pass. Type "{}" is not supported for target use "{}")",
                RenderTargetTypeNames[static_cast<u32>(renderTarget->getType())],
                RenderTargetTypeNames[static_cast<u32>(m_renderTargetUse)]);
            return *this;
        }
    }

    m_renderTargets.push_back({.target = renderTarget, .clearColor = clearColor});
    return *this;
}

u64 RenderPassBuilder::generateHash()
{
    u64 fnvPrime = 0x00000100000001b3;
    u64 hash = 0xcbf29ce484222325;
    auto combineHash = [&hash, fnvPrime](u64 val) { hash ^= val * fnvPrime; };
    auto u64Hash = std::hash<u64>();
    auto ptrHash = std::hash<void*>();

    combineHash(u64Hash(static_cast<u64>(m_type)));
    combineHash(u64Hash(static_cast<u64>(m_clearTargets)));
    combineHash(u64Hash(static_cast<u64>(m_renderTargetUse)));
    combineHash(u64Hash(static_cast<u64>(m_renderTargetClearUse)));
    combineHash(m_pipeline.generateHash());

    combineHash(u64Hash(m_inputs.size()));
    for (auto& in : m_inputs)
    {
        combineHash(u64Hash(static_cast<u64>(in.access)));
    }

    combineHash(u64Hash(m_outputs.size()));
    for (auto& out : m_outputs)
    {
        combineHash(u64Hash(static_cast<u64>(out.access)));
    }

    combineHash(u64Hash(m_renderTargets.size()));
    for (auto& renderTarget : m_renderTargets)
    {
        combineHash(ptrHash(renderTarget.target.get()));
        combineHash(u64Hash(static_cast<u64>(renderTarget.target.get()->getWidth())));
        combineHash(u64Hash(static_cast<u64>(renderTarget.target.get()->getHeight())));
    }

    return hash;
}

} // namespace huedra