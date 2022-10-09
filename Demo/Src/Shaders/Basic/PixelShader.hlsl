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
Texture2D DiffuseMap : register(t0); 

#define NUM_LIGHTS 3

float4 main(VertexOut pin) : SV_TARGET
{
    pin.NormalW = normalize(pin.NormalW);
    
    float3 toEyeVector = gEyePos - pin.PosW;
    float distToEye = length(toEyeVector);
    
    //normalize
    toEyeVector /= distToEye;
    
    LightingOutput results[NUM_LIGHTS];
    results[0] = ComputeDirectionalLight(gMaterial, gLight, pin.NormalW, toEyeVector);
    results[1] = ComputePointLight(gMaterial, gPointLight, pin.NormalW, pin.PosW, toEyeVector);
    results[2] = ComputeSpotLight(gMaterial, gSpotLight, pin.NormalW, pin.PosW, toEyeVector);
    
    LightingOutput result;
    result.Ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    result.Diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    result.Specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    [unroll]
    for (int i = 0; i < NUM_LIGHTS; ++i)
    {
        result.Ambient += results[i].Ambient;
        result.Diffuse += results[i].Diffuse;
        result.Specular += results[i].Specular;
    }
    
    float4 sampleColor = DiffuseMap.Sample(Sampler, pin.Tex);
    float4 final = sampleColor * (result.Ambient + result.Diffuse) + result.Specular;
    final.a = gMaterial.Diffuse.a * sampleColor.a;
    
    [flatten]
    if (gFog.FogEnabled > 0.0f)
    {
        float lerpFactor = saturate((distToEye - gFog.FogStart) / gFog.FogRange);
        final.rgb = lerp(final.rgb, gFog.FogColor.rgb, lerpFactor);
    }
    
    return final;
}