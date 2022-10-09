#include "Common.hlsli"

Texture2DArray TexArray : register(t0);
SamplerState Sampler : register(s0);


float4 main(GSOut pin) : SV_TARGET
{
    float3 uvw = float3(pin.TexCoord, pin.PrimID % 4);
    float4 sampleColor = TexArray.Sample(Sampler, uvw);

    clip(sampleColor.a - 0.2f);

    return sampleColor;
} 