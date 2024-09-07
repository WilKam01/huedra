#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uvs;
layout(location = 2) in vec3 normal;

layout(push_constant) uniform PushConstantData
{
    mat4 model;
} pushConstantData;

layout(binding = 0) uniform CameraBuffer
{
    mat4 projView;
} cameraBuffer;

layout(location = 0) out vec2 uv;
layout(location = 1) out vec3 fragColor;

void main() {
    gl_Position = cameraBuffer.projView * pushConstantData.model * vec4(position, 1.0);
    uv = uvs;
    fragColor = (normal + vec3(1)) / 2.0;
}