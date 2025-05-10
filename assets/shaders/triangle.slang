struct VertexOut
{
    float4 position : SV_Position;
    float4 color : COLOR0;
};

[shader("vertex")]
VertexOut vertMain(uint vertexID: SV_VertexID)
{
    float4 positions[3] = {
        float4(0.0, 0.5, 0.0, 1.0),   // Top
        float4(-0.5, -0.5, 0.0, 1.0), // Bottom Left
        float4(0.5, -0.5, 0.0, 1.0)   // Bottom Right
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

[shader("fragment")]
float4 fragMain(VertexOut in) : SV_Target { return in.color; }
