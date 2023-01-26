
cbuffer cbPerObject : register(b0)
{
	row_major float4x4 gWorldViewProj;
}

struct VertexIn
{
	float3 PosL : SV_POSITION;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
};


VertexOut main(VertexIn vin)
{
	VertexOut vout;

	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);

	return vout;
}