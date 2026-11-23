struct CameraBuffer 
{
    float4x4 viewProj;
};

struct VSInput 
{
    float3 position : POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL;
};

struct VSOutput 
{
    float3 fragPosition : TEXCOORD0;
    float2 fragUv : TEXCOORD1;
    float3 fragNormal : TEXCOORD2;
    float4 position : SV_Position;
};

[shader("vertex")]
VSOutput vertMain(VSInput input, uniform float4x4 model, ConstantBuffer<CameraBuffer> cameraBuffer) 
{
    VSOutput output;
    output.position = mul(cameraBuffer.viewProj, mul(model, float4(input.position, 1.0)));
    output.fragPosition = input.position;
    output.fragUv = input.uv;
    output.fragNormal = mul(cameraBuffer.viewProj, mul(model, float4(input.normal, 0.0))).xyz;
    return output;
}