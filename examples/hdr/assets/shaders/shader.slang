struct VSOutput
{
    float2 uv : UV;
    float4 position : SV_Position;
};

[shader("vertex")]
VSOutput vertMain(uint vertexIndex : SV_VertexID)
{
    VSOutput output;
    output.uv = float2((vertexIndex << 1) & 2, vertexIndex & 2);
    output.position = float4((output.uv * 2.0 + -float2(1.0)) * -1, 0.0, 1.0);
    return output;
}

[shader("fragment")]
float4 fragMain(VSOutput input, Texture2D hdr, SamplerState sampler) : SV_Target 
{
    float3 hdrColor = hdr.Sample(sampler, input.uv).rgb;

    hdrColor *= 1;

    // Reinhard tone mapping: compresses [0, infinity] -> [0, 1]
    float3 ldrColor = hdrColor / (hdrColor + float3(1.0));
    
    // Gamma correction
    ldrColor = pow(ldrColor, 1.0 / 2.2);

    return float4(ldrColor, 1.0);
}
