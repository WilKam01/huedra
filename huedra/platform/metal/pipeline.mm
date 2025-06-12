#include "pipeline.hpp"
#include "core/file/utils.hpp"
#include "core/global.hpp"
#include "core/log.hpp"
#include "graphics/pipeline_data.hpp"
#include "platform/metal/type_converter.hpp"
#include <Metal/Metal.h>
#include <optional>

namespace huedra {

void MetalPipeline::initGraphics(const PipelineBuilder& pipelineBuilder, id<MTLDevice> device)
{
    @autoreleasepool
    {
        m_device = device;
        m_builder = pipelineBuilder;

        std::map<ShaderStage, ShaderInput> shaders = pipelineBuilder.getShaderStages();
        std::vector<ShaderModule> shaderModules{};
        for (auto& [stage, input] : shaders)
        {
            if (std::ranges::find_if(shaderModules, [&input](ShaderModule& shaderModule) {
                    return input.shaderModule->getSlangModule() == shaderModule.getSlangModule();
                }) == shaderModules.end())
            {
                shaderModules.push_back(*input.shaderModule);
            }
        }

        m_shaderModule = global::graphicsManager.compileAndLinkShaderModules(shaderModules);

        NSError* error = nil;
        NSString* source = [[NSString alloc] initWithBytes:m_shaderModule.getCode().data()
                                                    length:m_shaderModule.getCode().size()
                                                  encoding:NSUTF8StringEncoding];

        id<MTLLibrary> library = [m_device newLibraryWithSource:source options:nil error:&error];
        if (library == nullptr && [[error localizedDescription] UTF8String] != nullptr)
        {
            log(LogLevel::ERR, "MetalPipeline::initGraphics(): Failed to create library from source: {}",
                [[error localizedDescription] UTF8String]);
            return;
        }

        std::vector<VertexInputStream> inputStreams{m_builder.getVertexInputStreams()};
        MTLVertexDescriptor* vertexDesc = [[MTLVertexDescriptor alloc] init];

        u32 locationOffset = 0;
        for (u32 i = 0; i < static_cast<u32>(inputStreams.size()); ++i)
        {
            vertexDesc.layouts[m_shaderModule.getVertexBufferOffsetMetal() + i].stride = inputStreams[i].size;
            vertexDesc.layouts[m_shaderModule.getVertexBufferOffsetMetal() + i].stepFunction =
                converter::convertVertexInputRate(inputStreams[i].inputRate);

            for (u32 j = 0; j < static_cast<u32>(inputStreams[i].attributes.size()); ++j)
            {
                vertexDesc.attributes[j + locationOffset].format =
                    converter::convertDataFormat(inputStreams[i].attributes[j].format);
                vertexDesc.attributes[j + locationOffset].offset = inputStreams[i].attributes[j].offset;
                vertexDesc.attributes[j + locationOffset].bufferIndex = m_shaderModule.getVertexBufferOffsetMetal() + i;
            }

            locationOffset += static_cast<u32>(inputStreams[i].attributes.size());
        }

        MTLRenderPipelineDescriptor* desc = [[MTLRenderPipelineDescriptor alloc] init];
        desc.vertexDescriptor = vertexDesc;

        desc.vertexFunction = [library
            newFunctionWithName:[NSString stringWithUTF8String:shaders[ShaderStage::VERTEX].entryPointName.c_str()]];
        m_functions.insert(std::pair<ShaderStage, id<MTLFunction>>(ShaderStage::VERTEX, desc.vertexFunction));
        if (shaders.contains(ShaderStage::FRAGMENT))
        {
            desc.fragmentFunction = [library
                newFunctionWithName:[NSString
                                        stringWithUTF8String:shaders[ShaderStage::FRAGMENT].entryPointName.c_str()]];
            m_functions.insert(std::pair<ShaderStage, id<MTLFunction>>(ShaderStage::FRAGMENT, desc.fragmentFunction));
        }

        desc.colorAttachments[0].pixelFormat = MTLPixelFormatRGBA8Unorm;

        desc.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;

        m_pipeline = [m_device newRenderPipelineStateWithDescriptor:desc error:&error];
        if (m_pipeline == nullptr && [[error localizedDescription] UTF8String] != nullptr)
        {
            log(LogLevel::ERR, "MetalPipeline::initGraphics(): Failed to create pipeline state: {}",
                [[error localizedDescription] UTF8String]);
            return;
        }

        m_parameterHandlers.resize(GraphicsManager::MAX_FRAMES_IN_FLIGHT);
        for (auto& parameterHandler : m_parameterHandlers)
        {
            parameterHandler.init(m_device, *this, m_shaderModule);
        }
    }
}

void MetalPipeline::cleanup()
{
    for (auto& parameterHandler : m_parameterHandlers)
    {
        parameterHandler.cleanup();
    }
    m_parameterHandlers.clear();

    for (auto& [stage, func] : m_functions)
    {
        [func release];
    }
    m_functions.clear();

    [m_pipeline release];
}

std::optional<id<MTLFunction>> MetalPipeline::getFunction(ShaderStage stage) const
{
    if (m_functions.contains(stage))
    {
        return m_functions.at(stage);
    }

    return std::nullopt;
}

MetalParameterHandler& MetalPipeline::getParameterHandler()
{
    return m_parameterHandlers[global::graphicsManager.getCurrentFrame()];
}

} // namespace huedra