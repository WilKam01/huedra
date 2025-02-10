#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;

layout(push_constant) uniform PushConstantData
{
    mat4 model;
} pushConstantData;

layout(set = 0, binding = 0) uniform CameraBuffer
{
    mat4 viewProj;
} cameraBuffer;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec2 fragUv;
layout(location = 2) out vec3 fragNormal;

void main() {
    gl_Position = cameraBuffer.viewProj * pushConstantData.model * vec4(position, 1.0);
    fragPosition = position;
    fragUv = uv;
    fragNormal = (cameraBuffer.viewProj * pushConstantData.model * vec4(normal, 0.0)).xyz;
}