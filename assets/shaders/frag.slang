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
PSOutput fragMain(PSInput input, Texture2D tex, SamplerState sampler) 
{
    PSOutput output;
    output.outColor = float4(tex.Sample(sampler, input.fragUv).rgb, 1.0);
    return output;
}