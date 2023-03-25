struct VertexOut
{
	float4 screenSpace : SV_POSITION;
	float2 texCoord : TEXCOORD;
};

cbuffer cbPerFrame : register(b0)
{
	float3 lightDir;
	float pad1;
	float3 lightColor;
	float pad2;
}

Texture2D Normal	: register(t0);
Texture2D Diffuse		: register(t1);
Texture2D Position	: register(t2);

float4 main(VertexOut pin) : SV_TARGET
{
	int3 sampleIndex = int3(pin.screenSpace.xy, 0);

	float3 normal = Normal.Load(sampleIndex).xyz;
	float4 posNshadow = Position.Load(sampleIndex);
	float3 diffuse = Diffuse.Load(sampleIndex).xyz;

	float3 L = -lightDir;

	normal = normalize(normal);

	float lightAmountDL = saturate(dot(normal, L));
	float3 color = lightColor * lightAmountDL * diffuse * posNshadow.w;

	
	/*float gamma = 2.2f;
	color.r = pow(color.r, 1.0f / gamma);
	color.g = pow(color.g, 1.0f / gamma);
	color.b = pow(color.b, 1.0f / gamma);*/

	return float4(color, 1.0f);
}