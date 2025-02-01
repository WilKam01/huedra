#version 450

layout(set = 0, binding = 1) uniform sampler2D tex;

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec2 fragUv;
layout(location = 2) in vec3 fragNormal;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outAlbedo;

void main() {
    outPosition = vec4(fragPosition, 1.0);
    outNormal = vec4(fragNormal, 0.0);
    outAlbedo = vec4(texture(tex, fragUv).rgb, 1.0);
}