#include <metal_stdlib>
using namespace metal;

struct VertexOut {
    float4 position [[position]];
    float4 color;
};

// Vertex Shader
vertex VertexOut vertex_main(uint vertexID [[vertex_id]]) {
    float4 positions[3] = {
        float4( 0.0,  0.5, 0.0, 1.0),  // Top
        float4(-0.5, -0.5, 0.0, 1.0),  // Bottom Left
        float4( 0.5, -0.5, 0.0, 1.0)   // Bottom Right
    };

    float4 colors[3] = {
        float4(1.0, 0.0, 0.0, 1.0), // Red
        float4(0.0, 1.0, 0.0, 1.0), // Green
        float4(0.0, 0.0, 1.0, 1.0)  // Blue
    };

    VertexOut out;
    out.position = positions[vertexID];
    out.color = colors[vertexID];
    return out;
}

// Fragment Shader
fragment float4 fragment_main(VertexOut in [[stage_in]]) {
    return in.color;
}