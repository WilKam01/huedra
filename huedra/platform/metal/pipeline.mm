#include "pipeline.hpp"
#include "core/global.hpp"
#include "core/log.hpp"
#include "graphics/pipeline_data.hpp"

namespace huedra {

void MetalPipeline::initGraphics(const PipelineBuilder& pipelineBuilder, id<MTLDevice> device)
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

    id<MTLFunction> frag = [library newFunctionWithName:@"frag_main"];
    MTLRenderPipelineDescriptor* desc = [[MTLRenderPipelineDescriptor alloc] init];

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

void MetalPipeline::cleanup() { [m_pipeline release]; }

} // namespace huedra