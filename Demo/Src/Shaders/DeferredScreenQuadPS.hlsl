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

	float width;
	float height;
	float hamN;
	float useAO;

	float4 hammersley[48];
}

Texture2D Normal	: register(t0);
Texture2D Diffuse		: register(t1);
Texture2D Position	: register(t2);
Texture2D PBR : register(t3);

TextureCube IRMap : register(t4);
TextureCube EnvMap : register(t5);

Texture2D AOMap : register(t6);

SamplerState Sampler : register(s0);

float4 main(VertexOut pin) : SV_TARGET
{
	int3 sampleIndex = int3(pin.screenSpace.xy, 0);

	float3 normal = Normal.Load(sampleIndex).xyz;
	float4 posNshadow = Position.Load(sampleIndex);
	float4 PBRData = PBR.Load(sampleIndex);

	float metallic = PBRData.x;
	float roughness = PBRData.y;
    float ao = 1.0f;
	
    if (useAO > 0.0f)
        ao = AOMap.Load(sampleIndex).x;
	
	float gammaExposure = PBRData.w;

	float4 diff = Diffuse.Load(sampleIndex);
	float3 albedo = pow(diff.xyz, gammaExposure);

	if (diff.a < 0.0f)
		return float4(albedo, 1.0f);

	float3 N = normalize(normal);
	float3 V = normalize(cameraPosition - posNshadow.xyz);

	float3 F0 = float3(0.04, 0.04, 0.04);
	F0 = lerp(F0, albedo, metallic);

	float3 Lo = float3(0.0, 0.0f, 0.0);

	float3 L = -lightDir; 
	float3 H = normalize(V + L);
	
	float3 specular = float3(0,0,0);

	float3 kS = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
	float3 kD = float3(1.0, 1.0f, 1.0f) - kS;
	kD *= 1.0 - metallic;


	float3 irradiance = IRMap.Sample(Sampler, N).rgb;
	float3 diffuse = (kD * albedo / 3.14159265359) * irradiance;


	float3 R = 2 * dot(N, V) * N - V;
	float3 A = normalize(cross(float3(0, 0, 1), R));
	float3 B = normalize(cross(R, A));

	for (int i = 0; i < hamN * 2; i += 2)
	{
		float val1 = ((float[4])(hammersley[i / 4]))[i % 4];

		float theta = atan(roughness * sqrt(val1) / sqrt(1 - val1));

		float2 currUV = float2(val1, theta / 3.14159265359);

		float3 currL = float3(
			cos(2 * 3.14159265359 * (0.5 - currUV.x)) * sin(3.14159265359 * currUV.y),
			sin(2 * 3.14159265359 * (0.5 - currUV.x)) * sin(3.14159265359 * currUV.y),
			cos(3.14159265359 * currUV.y)
		);

		
		float3 wK = normalize(currL.x * A + currL.y * B + currL.z * R);
		H = normalize(wK + V);

		float level = 0.5 * log2(width * height / hamN) - 0.5 * log2(DistributionGGX(H, N, roughness));

		float3 Li = EnvMap.SampleLevel(Sampler, wK, level).rgb;

		specular += cos(theta) * Li * GeometrySmith(N, V, wK, roughness) * fresnelSchlick(max(dot(wK, H), 0.0), kS) / (4.0 * max(dot(N, V), 0.0) * max(dot(N, wK), 0.0) + 0.0001);
	}
	specular /= hamN;

	Lo += (diffuse + specular) * ao;

	float3 ambient = float3(0.03, 0.03, 0.03) * albedo;

	float3 color = ambient;

	color = Lo;

	color = color / (color + float3(1.0f, 1.0f, 1.0f));
	color = pow(color, 1.0f / gammaExposure);

	return float4(color, 1.0f);
}