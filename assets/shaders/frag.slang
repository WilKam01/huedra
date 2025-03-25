[[vk::binding(1, 0)]]
Sampler2D tex;

struct PSInput 
{
    float3 fragPosition : TEXCOORD0;
    float2 fragUv : TEXCOORD1;
    float3 fragNormal : TEXCOORD2;
};

struct PSOutput 
{
    float4 outColor : SV_Target;
};

[shader("fragment")]
PSOutput main(PSInput input) 
{
    PSOutput output;
    output.outColor = float4(tex.Sample(input.fragUv).rgb, 1.0);
    return output;
}