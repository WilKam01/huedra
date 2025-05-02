struct CameraBuffer 
{
    float4x4 viewProj;
};

struct Globals 
{
    struct HelloWorld 
    {
        ConstantBuffer<CameraBuffer> camerBuffer;
        uniform float4x4 worldMat;
    };
    HelloWorld hello;
    
    struct Test 
    {
        float testFloat;
        ConstantBuffer<float4x4> mat;
        Texture2D tex;
    };
    ParameterBlock<Test> testData;
};

Globals global;

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
    output.position = mul(cameraBuffer.viewProj, mul(global.hello.worldMat, float4(input.position, global.testData.testFloat)));
    output.fragPosition = input.position;
    output.fragUv = input.uv;
    output.fragNormal = mul(cameraBuffer.viewProj, mul(model, float4(input.normal, 0.0))).xyz;
    return output;
}

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

Texture2D teax;
SamplerState samplesr;

struct PSResources
{
    struct Textures 
    {
        Texture2D tex;
    };
    ParameterBlock<Textures> textures;
    SamplerState sampler;
};

struct PSResources2
{
    Texture2D texture;
    SamplerState sampler;
};

[shader("fragment")]
PSOutput fragMain(PSInput input, ParameterBlock<PSResources2> resources2, ParameterBlock<PSResources> resources) 
{
    PSOutput output;

    output.outColor = float4(resources.textures.tex.Sample(resources.sampler, input.fragUv).rgb + resources2.texture.Sample(resources.sampler, input.fragUv).rgb, 1.0);
    return output;
}