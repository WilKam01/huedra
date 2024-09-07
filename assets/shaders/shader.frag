#version 450

layout(set = 0, binding = 1) uniform sampler2D tex;

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor + texture(tex, uv).rgb * 0.1, 1.0);
}