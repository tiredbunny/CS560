#include "Lighting.hlsli"

struct VertexOut
{
	float4 PixelCoordinates : SV_POSITION;
};

cbuffer LightData : register(b0)
{
	float3 cameraPosition;
	float pad1;

	float3 lightColor;
	float visualizeSphere;

	float3 lightPosition;
	float radius;
}

Texture2D Normal	: register(t0);
Texture2D Diffuse		: register(t1);
Texture2D Position	: register(t2);
Texture2D PBR : register(t3);

float4 main(VertexOut pin) : SV_TARGET
{
	return float4(0, 0, 0, 1.0f);

	int3 sampleIndex = int3(pin.PixelCoordinates.xy, 0);
	 
	float3 normal = Normal.Load(sampleIndex).xyz;
	float3 position = Position.Load(sampleIndex).xyz;
	float4 PBRData = PBR.Load(sampleIndex);

	float metallic = PBRData.x;
	float roughness = PBRData.y;
	float ao = PBRData.z;
	float gammaExposure = PBRData.w;

	float3 diffuse = pow(Diffuse.Load(sampleIndex).xyz, gammaExposure);
	float objectID = Diffuse.Load(sampleIndex).w;

	float3 L = lightPosition - position;
	float dist = length(L);

	if (dist > radius || objectID == 0.4f)
	{
		return float4(0.0f, 0.0f, 0.0f, 0.0f);
	}

	L /= dist;

	float3 N = normalize(normal);
	float3 V = normalize(cameraPosition - position);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
	float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, diffuse, metallic);

    // reflectance equation
	float3 Lo = float3(0.0, 0.0f, 0.0);

	// calculate per-light radiance
	float3 H = normalize(V + L);
	float attenuation = 1.0f / (dist)-1.0f / (radius); // 1.0 / (distance * distance);
	float3 radiance = lightColor * attenuation;

	// Cook-Torrance BRDF
	float NDF = DistributionGGX(N, H, roughness);
	float G = GeometrySmith(N, V, L, roughness);
	float3 F = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

	float3 numerator = NDF * G * F;
	float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
	float3 specular = numerator / denominator;

	// kS is equal to Fresnel
	float3 kS = F;
	// for energy conservation, the diffuse and specular light can't
	// be above 1.0 (unless the surface emits light); to preserve this
	// relationship the diffuse component (kD) should equal 1.0 - kS.
	float3 kD = float3(1.0, 1.0f, 1.0f) - kS;
	// multiply kD by the inverse metalness such that only non-metals 
	// have diffuse lighting, or a linear blend if partly metal (pure metals
	// have no diffuse light).
	kD *= 1.0 - metallic;

	// scale light by NdotL
	float NdotL = max(dot(N, L), 0.0);

	// add to outgoing radiance Lo
	Lo += (kD * diffuse / 3.14159265359 + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again


    // ambient lighting (note that the next IBL tutorial will replace 
    // this ambient lighting with environment lighting).
	float3 ambient = float3(0.03, 0.03, 0.03) * diffuse * ao;
	float3 color = ambient + Lo;

	color = color / (color + float3(1.0f, 1.0f, 1.0f));
	color = pow(color, 1.0f / gammaExposure);



	return float4(color, 1.0f);
}