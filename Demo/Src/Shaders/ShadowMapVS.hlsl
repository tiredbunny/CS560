struct VertexIn
{
	float3 PosL : SV_POSITION;
	float3 NormalL : NORMAL;
	float2 TexCoord : TEXCOORD;
};

struct VertexOut
{ 
	float4 PosH : SV_POSITION;
};

cbuffer cbPerObject : register(b0)
{
	row_major float4x4 gWorldViewProj;
};

VertexOut main(VertexIn vin)
{
	VertexOut vout;

	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);

	return vout;
}