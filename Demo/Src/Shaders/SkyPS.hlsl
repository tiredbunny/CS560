
struct VertexOut
{
	float4 PosH : SV_POSITION;
	float3 PosL : POSITION;
};

cbuffer cbPerFrame : register(b0)
{
	float2 ScreenResolution;
	float2 padding;
	
	//for gradient
	float4 ColorA;
	float4 ColorB;
};

TextureCube gCubeMap : register(t0);
SamplerState gSampler : register(s0);

float4 main(VertexOut pin) : SV_Target
{
	 float2 st = pin.PosH.xy / ScreenResolution;

	 float mixValue = distance(st, float2(0, 1));
	 float4 color = lerp(ColorA, ColorB, mixValue);

	 if (padding.x == 1.0f)
		 return gCubeMap.Sample(gSampler, pin.PosL);

	return color;
}