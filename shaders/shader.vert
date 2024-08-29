#version 450

layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;

layout(binding = 0) uniform CameraBuffer
{
    mat4 projView;
} cameraBuffer;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = cameraBuffer.projView * vec4(position, 0.0, 1.0);
    fragColor = color;
}