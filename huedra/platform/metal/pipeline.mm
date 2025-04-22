#include "pipeline.hpp"
#include "core/log.hpp"
#include <Metal/Metal.h>

namespace huedra {

void MetalPipeline::initGraphics(const PipelineBuilder& pipelineBuilder, id<MTLDevice> device)
{
    m_device = device;

    NSError* error = nil;
    NSString* source = [NSString stringWithContentsOfFile:@"assets/shaders/shader.metal"
                                                 encoding:NSUTF8StringEncoding
                                                    error:&error];

    id<MTLLibrary> library = [m_device newLibraryWithSource:source options:nil error:&error];
    id<MTLFunction> vert = [library newFunctionWithName:@"vertex_main"];
    id<MTLFunction> frag = [library newFunctionWithName:@"fragment_main"];

    MTLRenderPipelineDescriptor* desc = [[MTLRenderPipelineDescriptor alloc] init];
    desc.vertexFunction = vert;
    desc.fragmentFunction = frag;
    desc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;

    m_pipeline = [m_device newRenderPipelineStateWithDescriptor:desc error:&error];
    if (m_pipeline == nullptr && [[error localizedDescription] UTF8String] != nullptr)
    {
        log(LogLevel::ERR, "Failed to create pipeline state: {}", [[error localizedDescription] UTF8String]);
        return;
    }
}

void MetalPipeline::cleanup() { [m_pipeline release]; }

} // namespace huedra