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
    
    // Complete projection by doing division by w.
    pin.ShadowPosH.xyz /= pin.ShadowPosH.w;

    float depth = pin.ShadowPosH.z;
    float4 lightDepth = ShadowMap.Sample(Sampler, pin.ShadowPosH.xy);

    float fPercentLit = 1.0f - Hamburger4MSMMethod(lightDepth, depth);
    //float  fAvgZ = lightDepth.x; // Filtered z
    //float  fAvgZ2 = lightDepth.y; // Filtered z-squared

    //float fPercentLit = 0.0f;
    //if (depth <= fAvgZ) // We put the z value in w so that we can index the texture array with Z.
    //{
    //    fPercentLit = 1.0f;
    //}
    //else
    //{
    //    float variance = (fAvgZ2)-(fAvgZ * fAvgZ);
    //    variance = min(1.0f, max(0.0f, variance + 0.00001f));

    //    float mean = fAvgZ;
    //    float d = depth - mean; // We put the z value in w so that we can index the texture array with Z.
    //    float p_max = variance / (variance + d * d);

    //    // To combat light-bleeding, experiment with raising p_max to some power
    //    // (Try values from 0.1 to 100.0, if you like.)
    //    fPercentLit = pow(p_max, 4);

    //}

    PixelShaderOutput output;
    
    output.Normal = float4(pin.NormalW, 1.0f);
    output.Diffuse = DiffuseMap.Sample(Sampler, pin.Tex);
    output.Position = float4(pin.PosW, fPercentLit);


    return output;
}