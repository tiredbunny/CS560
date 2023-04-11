#include "Lighting.hlsli"

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
	float3 cameraPosition;
	float pad3;
}

Texture2D Normal	: register(t0);
Texture2D Diffuse		: register(t1);
Texture2D Position	: register(t2);
Texture2D PBR : register(t3);

TextureCube IRMap : register(t4);
SamplerState Sampler : register(s0);

float4 main(VertexOut pin) : SV_TARGET
{
	int3 sampleIndex = int3(pin.screenSpace.xy, 0);

	float3 normal = Normal.Load(sampleIndex).xyz;
	float4 posNshadow = Position.Load(sampleIndex);
	float4 PBRData = PBR.Load(sampleIndex);

	float metallic = PBRData.x;
	float roughness = PBRData.y;
	float ao = PBRData.z;
	float gammaExposure = PBRData.w;

	float4 diff = Diffuse.Load(sampleIndex);
	float3 albedo = pow(diff.xyz, gammaExposure);

	if (diff.a == 0.4f)
		return float4(albedo, 1.0f);

	float3 N = normalize(normal);
	float3 V = normalize(cameraPosition - posNshadow.xyz);

	float3 F0 = float3(0.04, 0.04, 0.04);
	F0 = lerp(F0, albedo, metallic);

	float3 Lo = float3(0.0, 0.0f, 0.0);

	// calculate per-light radiance
	float3 L = -lightDir; 
	float3 H = normalize(V + L);

	float3 radiance = lightColor;

	// Cook-Torrance BRDF
	float NDF = DistributionGGX(N, H, roughness);
	float G = GeometrySmith(N, V, L, roughness);
	float3 F = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

	float3 numerator = NDF * G * F;
	float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; 
	float3 specular = numerator / denominator;


	float3 kS = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
	float3 kD = float3(1.0, 1.0f, 1.0f) - kS;
	kD *= 1.0 - metallic;
	float3 irradiance = IRMap.Sample(Sampler, N).rgb;
	float3 diffuse = irradiance * albedo;

	float NdotL = max(dot(N, L), 0.0);

	Lo += (kD * diffuse / 3.14159265359 + specular) * radiance * NdotL;  

	float3 ambient = (kD * diffuse + specular) * ao; 
	float3 color = ambient + Lo;

	color = color / (color + float3(1.0f, 1.0f, 1.0f));
	color = pow(color, 1.0f / gammaExposure);

	return float4(color, 1.0f);
}