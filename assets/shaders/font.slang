struct VSInput
{
    float3 mat0 : MATRIX0;
    float3 mat1 : MATRIX1;
    float3 mat2 : MATRIX2;
};

struct VSOutput
{
    float4 position : SV_Position;
};

struct Info
{
    float2 position;
    float2 size;
    float4x4 projection;
};

[shader("vertex")]
VSOutput vertMain(VSInput input, ConstantBuffer<Info> info, uint vertexID: SV_VertexID, uint instanceID: SV_InstanceID)
{
    VSOutput output;
    float2 positions[4] = { float2(0.0f, 0.0f), float2(1.0f, 0.0f), float2(0.0f, 1.0f), float2(1.0f, 1.0f) };
    float3x3 model =
        float3x3(input.mat0, input.mat1, (input.mat2 * float3(info.size, 1.0f)) + float3(info.position, 0.0f));
    output.position =
        mul(info.projection, float4(mul(float3(positions[vertexID] * info.size, 1.0f), model).xy, 0.0f, 1.0f));
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
