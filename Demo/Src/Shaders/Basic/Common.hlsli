#include "..\Lighting.hlsli"

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : WORLD_POS;
    float3 NormalW : NORMAL0;
    float2 Tex : TEXCOORD0;
    float4 ShadowPosH : TEXCOORD1;
};


cbuffer cbPerObject : register(b0)
{
    row_major float4x4 gWorldViewProj;
    row_major float4x4 gWorld;
    row_major float4x4 gWorldInverseTranspose;
    row_major float4x4 gTextureTransform;
    row_major float4x4 gShadowTransform;

    Material gMaterial;
};

//---------------------------------------------------------------------------------------
// Performs shadowmap test to determine if a pixel is in shadow.
//---------------------------------------------------------------------------------------

static const float SMAP_SIZE = 2048.0f;
static const float SMAP_DX = 1.0f / SMAP_SIZE;

float CalcShadowFactor(SamplerComparisonState samShadow,
	Texture2D shadowMap,
	float4 shadowPosH)
{
	// Complete projection by doing division by w.
	shadowPosH.xyz /= shadowPosH.w;

	// Depth in NDC space.
	float depth = shadowPosH.z;

	// Texel size.
	const float dx = SMAP_DX;

	float percentLit = 0.0f;
	const float2 offsets[9] =
	{
		float2(-dx,  -dx), float2(0.0f,  -dx), float2(dx,  -dx),
		float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
		float2(-dx,  +dx), float2(0.0f,  +dx), float2(dx,  +dx)
	};

	[unroll]
	for (int i = 0; i < 9; ++i)
	{
		percentLit += shadowMap.SampleCmpLevelZero(samShadow,
			shadowPosH.xy + offsets[i], depth).r;
	}

	return percentLit /= 9.0f;
}
