
struct VertexOut
{
	float4 PosH : SV_POSITION;
	float3 PosL : POSITION;
};


TextureCube gCubeMap : register(t0);
SamplerState gSampler : register(s0);

float4 main(VertexOut pin) : SV_Target1
{
	return float4(gCubeMap.Sample(gSampler, pin.PosL).xyz, -1.0f);
}