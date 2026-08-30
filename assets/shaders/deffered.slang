struct VertInput
{
    float3 position : POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL;
};

struct VertOutput
{
    float3 worldPosition : TEXCOORD0;
    float2 uv : TEXCOORD1;
    float3 normal : TEXCOORD2;
    float4 position : SV_Position;
    float pointSize : SV_PointSize;
};

[shader("vertex")]
VertOutput vertMain(VertInput input, ConstantBuffer<float4x4> cameraMatrix, uniform float4x4 modelMatrix)
{
    VertOutput output;
    output.position = mul(cameraMatrix, mul(modelMatrix, float4(input.position, 1.0)));
    output.worldPosition = input.position;
    output.uv = input.uv;
    output.normal = mul(cameraMatrix, mul(modelMatrix, float4(input.normal, 0.0))).xyz;
    output.pointSize = 5.0f;
    return output;
}

struct FragInput
{
    float3 worldPosition : TEXCOORD0;
    float2 uv : TEXCOORD1;
    float3 normal : TEXCOORD2;
};

struct FragOutput
{
    float4 worldPosition : SV_Target0;
    float4 normal : SV_Target1;
    float4 albedo : SV_Target2;
};

struct Resources
{
    Texture2D texture;
    SamplerState sampler;
};

[shader("fragment")]
FragOutput fragMain(FragInput input, ParameterBlock<Resources> resources)
{
    FragOutput output;
    output.worldPosition = float4(input.worldPosition, 1.0);
    output.normal = float4(input.normal, 1.0);
    output.albedo = float4(resources.texture.Sample(resources.sampler, input.uv).rgb, 1.0);
    return output;
}

struct GBuffers
{
    Texture2D position;
    Texture2D normal;
    Texture2D albedo;
};

struct Light
{
    float4 position;
    float4 color;
};

[shader("compute")]
[numthreads(16, 16, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID, ConstantBuffer<Light> lightBuffer,
                 ParameterBlock<GBuffers> gBuffers, RWTexture2D outputTexture)
{
    int2 pixelCoords = int2(dispatchThreadID.xy);

    float3 position = gBuffers.position[pixelCoords].rgb;
    float3 normal = gBuffers.normal[pixelCoords].rgb;
    float3 albedo = gBuffers.albedo[pixelCoords].rgb;

    float3 lightDir = normalize(lightBuffer.position.xyz - position);
    float diff = max(dot(normal, lightDir), 0.0);
    float3 diffuse = diff * lightBuffer.color.rgb * albedo * lightBuffer.color.a;

    float3 ambient = 0.3 * albedo;

    float3 finalColor = ambient + diffuse;
    outputTexture[pixelCoords] = float4(finalColor, 1.0);
}
