#include "..\Lighting.hlsli"

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : WORLD_POS;
    float3 NormalW : NORMAL0;
    float2 Tex : TEXCOORD0;
};


cbuffer cbPerObject : register(b0)
{
    row_major float4x4 gWorldViewProj;
    row_major float4x4 gWorld;
    row_major float4x4 gWorldInverseTranspose;
    row_major float4x4 gTextureTransform;
    Material gMaterial;
};
