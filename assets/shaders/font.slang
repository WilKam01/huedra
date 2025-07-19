struct VSInput
{
    float2 position;
};

struct VSOutput
{
    float pointSize : SV_PointSize;
    float4 position : SV_Position;
};

struct Info
{
    float2 position;
    float2 size;
    float4x4 projection;
};

[shader("vertex")]
VSOutput vertMain(VSInput input, ConstantBuffer<Info> info, uint vertexID: SV_VertexID, uniform float offset)
{
    VSOutput output;
    output.position =
        mul(info.projection, float4(input.position * info.size + info.position + float2(offset, 0.0f), 0.0f, 1.0f));
    output.pointSize = 2.0f;
    return output;
}

struct FSInput
{};

struct FSOutput
{
    float4 outColor : SV_Target;
};

[shader("fragment")]
FSOutput fragMain(FSInput input)
{
    FSOutput output;
    output.outColor = float4(1.0f);
    return output;
}
