struct VertexIn
{
    float2 position : POSITION;
    float3 color : COLOR;
}

struct VertexOut
{
    float4 position : SV_Position;
    float4 color : COLOR0;
};

[shader("vertex")]
VertexOut vertMain(VertexIn in)
{
    VertexOut out;
    out.position = float4(in.position, 0.0, 1.0);
    out.color = float4(in.color, 1.0);
    return out;
}

[shader("fragment")]
float4 fragMain(VertexOut in) : SV_Target { return in.color; }
