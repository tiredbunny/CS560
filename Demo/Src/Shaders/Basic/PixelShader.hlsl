#include "Common.hlsli"

//register b0 is already used by cbPerObject from Common.hlsli

cbuffer cbPerFrame : register(b1)
{
    DirectionalLight gLight;
    PointLight gPointLight;
    SpotLight gSpotLight;
    
    float3 gEyePos; 
    float pad;
   
    FogProperties gFog;
};

 
SamplerState Sampler : register(s0);
SamplerComparisonState SamplerShadow : register(s1);


Texture2D DiffuseMap : register(t0); 
Texture2D ShadowMap : register(t1);

struct PixelShaderOutput
{
    float4 Normal				: SV_Target0;			
    float4 Diffuse				: SV_Target1;			
    float4 Position				: SV_Target2;
};

PixelShaderOutput main(VertexOut pin) : SV_TARGET
{
    pin.NormalW = normalize(pin.NormalW);
    
    float shadow = CalcShadowFactor(SamplerShadow, ShadowMap, pin.ShadowPosH);

    PixelShaderOutput output;
    
    output.Normal = float4(pin.NormalW, 1.0f);
    output.Diffuse = DiffuseMap.Sample(Sampler, pin.Tex);
    output.Position = float4(pin.PosW, shadow);


    return output;
}