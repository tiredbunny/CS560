#include "Common.hlsli"

cbuffer cbSkinned : register(b1)
{
    row_major float4x4 gBoneTransforms[96];
};

struct VertexIn
{
    float3 PosL : SV_POSITION;
    float3 NormalL : NORMAL0;
    float2 Tex : TEXCOORD0;
    float3 Weights    : WEIGHTS;
    uint4 BoneIndices : BONEINDICES;
};  


VertexOut main( VertexIn vin ) 
{ 
    VertexOut vout;

	// Init array or else we get strange warnings about SV_POSITION.
	float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	weights[0] = vin.Weights.x;
	weights[1] = vin.Weights.y;
	weights[2] = vin.Weights.z;
	weights[3] = 1.0f - weights[0] - weights[1] - weights[2];

	float3 posL = float3(0.0f, 0.0f, 0.0f);
	float3 normalL = float3(0.0f, 0.0f, 0.0f);

	for (int i = 0; i < 4; ++i)
	{
		// Assume no nonuniform scaling when transforming normals, so 
		// that we do not have to use the inverse-transpose.

		posL += weights[i] * mul(float4(vin.PosL, 1.0f), gBoneTransforms[vin.BoneIndices[i]]).xyz;
		normalL += weights[i] * mul(vin.NormalL, (float3x3)gBoneTransforms[vin.BoneIndices[i]]);
	}
    
    vout.PosW = mul(float4(posL, 1.0f), gWorld).xyz;  
    vout.PosH = mul(float4(posL, 1.0f), gWorldViewProj);
    vout.NormalW = mul(normalL, (float3x3) gWorldInverseTranspose);

    vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTextureTransform).xy;
    
    return vout;
}