#include "pipeline.hpp"
#include "core/file/utils.hpp"
#include "core/global.hpp"
#include "core/log.hpp"
#include "graphics/pipeline_data.hpp"
#include "platform/metal/type_converter.hpp"

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
            vertexDesc.layouts[i].stride = inputStreams[i].size;
            vertexDesc.layouts[i].stepFunction = converter::convertVertexInputRate(inputStreams[i].inputRate);

            for (u32 j = 0; j < static_cast<u32>(inputStreams[i].attributes.size()); ++j)
            {
                vertexDesc.attributes[j + locationOffset].format =
                    converter::convertDataFormat(inputStreams[i].attributes[j].format);
                vertexDesc.attributes[j + locationOffset].offset = inputStreams[i].attributes[j].offset;
                vertexDesc.attributes[j + locationOffset].bufferIndex = i;
            }

            locationOffset += static_cast<u32>(inputStreams[i].attributes.size());
        }

        MTLRenderPipelineDescriptor* desc = [[MTLRenderPipelineDescriptor alloc] init];
        desc.vertexDescriptor = vertexDesc;

        desc.vertexFunction = [library
            newFunctionWithName:[NSString stringWithUTF8String:shaders[ShaderStage::VERTEX].entryPointName.c_str()]];
        if (shaders.contains(ShaderStage::FRAGMENT))
        {
            desc.fragmentFunction = [library
                newFunctionWithName:[NSString stringWithUTF8String:shaders[ShaderStage::FRAGMENT].entryPointName.c_str()]];
        }

        desc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;

        m_pipeline = [m_device newRenderPipelineStateWithDescriptor:desc error:&error];
        if (m_pipeline == nullptr && [[error localizedDescription] UTF8String] != nullptr)
        {
            log(LogLevel::ERR, "MetalPipeline::initGraphics(): Failed to create pipeline state: {}",
                [[error localizedDescription] UTF8String]);
            return;
        }
    }
}

void MetalPipeline::cleanup() { [m_pipeline release]; }

} // namespace huedra