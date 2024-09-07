#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

layout(push_constant) uniform PushConstantData
{
    mat4 model;
} pushConstantData;

layout(binding = 0) uniform CameraBuffer
{
    mat4 projView;
} cameraBuffer;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = cameraBuffer.projView * pushConstantData.model * vec4(position, 1.0);
    fragColor = color;
}