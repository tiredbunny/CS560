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

float4 main(VertexOut pin) : SV_TARGET
{
	int3 sampleIndex = int3(pin.PixelCoordinates.xy, 0);
	 
	float3 normal = Normal.Load(sampleIndex).xyz;
	float3 position = Position.Load(sampleIndex).xyz;
	float3 diffuse = Diffuse.Load(sampleIndex).xyz;


	float3 L = lightPosition - position;
	float dist = length(L);


	if (dist > radius)
	{
		return float4(0.0f, 0.0f, 0.0f, 0.0f);
	}

	L /= dist;

	float att = 1.0f / (dist) - 1.0f / (radius);


	float lightAmount = saturate(dot(normal, L));
	float3 color = lightAmount * lightColor * att;

	//Specular calc
	float3 V = cameraPosition - position;
	float3 H = normalize(L + V);
	float specular = pow(saturate(dot(normal, H)), 10) * att;

	float3 finalDiffuse = color * diffuse;
	float3 finalSpecular = specular * diffuse * att;

	float4 totalColor = float4(finalDiffuse, 1.0f);

	float gamma = 1.2f;
	totalColor.r = pow(totalColor.r, 1.0f / gamma);
	totalColor.g = pow(totalColor.g, 1.0f / gamma);
	totalColor.b = pow(totalColor.b, 1.0f / gamma);

	if (visualizeSphere == 1.0f)
		return float4(1.0f, 0.0f, 0.0f, 1.0f);
	else
		return totalColor;
}