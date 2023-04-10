#include "Shadows.hlsli"


cbuffer cbPerFrame : register(b0)
{
    float ShadowMethod;
    float3 pad;

    float metallic;
    float roughness;
    float ao;
    float gammaExposure;
};


SamplerState Sampler : register(s0);
SamplerComparisonState SamplerShadow : register(s1);
Texture2D DiffuseMap : register(t0); 
Texture2D ShadowMap : register(t1);

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : WORLD_POS;
    float3 NormalW : NORMAL0;
    float2 Tex : TEXCOORD0;
    float4 ShadowPosH : TEXCOORD1;
};


struct PixelShaderOutput
{
    float4 Normal				: SV_Target0;			
    float4 Diffuse				: SV_Target1;			
    float4 Position				: SV_Target2;
    float4 PBRData              : SV_Target3;
};

PixelShaderOutput main(VertexOut pin) : SV_TARGET
{
    pin.NormalW = normalize(pin.NormalW);
    
    // Complete projection by doing division by w.
    pin.ShadowPosH.xyz /= pin.ShadowPosH.w;

    float depth = pin.ShadowPosH.z;
    float4 lightDepth = ShadowMap.Sample(Sampler, pin.ShadowPosH.xy);

    float fPercentLit = 0.0f;

    if (ShadowMethod == 1.0f)
        fPercentLit = 1.0f - Hamburger4MSMMethod(lightDepth, depth);
    else
        fPercentLit = VarianceMethod(lightDepth, depth);

    PixelShaderOutput output;
    output.Normal = float4(pin.NormalW, 1.0f);
    output.Diffuse = DiffuseMap.Sample(Sampler, pin.Tex);
    output.Position = float4(pin.PosW, fPercentLit);
    output.PBRData = float4(metallic, roughness, ao, gammaExposure);

    return output;
}