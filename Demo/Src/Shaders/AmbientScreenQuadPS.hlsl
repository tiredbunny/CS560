struct VertexOut
{
    float4 screenSpace : SV_POSITION;
    float2 texCoord : TEXCOORD;
};
static const float PI = 3.141592;

Texture2D Normal : register(t0);
Texture2D Diffuse : register(t1);
Texture2D Position : register(t2);
Texture2D PBR : register(t3);

cbuffer cbPerFrame : register(b0)
{
    float3 eyePos;
    float s;
	
    float width;
    float height;
    float k;
    float R;
}

SamplerState Sampler : register(s0);

float4 main(VertexOut pin) : SV_TARGET
{
	int n = 10;
	float c = 0.1 * R;
	float delta = 0.001;

    int2 integerCoeff = pin.screenSpace.xy;
    float2 floatingCoords = pin.screenSpace.xy / float2(width, height);
	
    int3 sampleIndex = int3(pin.screenSpace.xy, 0);
	
    float3 N = Normal.Load(sampleIndex).xyz;
    float3 P = Position.Load(sampleIndex).xyz;
	float d = length(eyePos - P.xyz);
	
	float phi = (30 * integerCoeff.x) ^ integerCoeff.y + 10 * integerCoeff.x * integerCoeff.y;
	float sum = 0;
	for(int i = 0; i < n; ++i)
	{
		float alpha = (i + 0.5f)/n;
		float h = alpha * R/d;
		float theta = 2 * PI * alpha * (7.0 * n / 9.0) + phi;

        //int3 integerCoords = int3((floatingCoords + h * float2(cos(theta), sin(theta))) * float2(width, height), 0.0f);
		
        float3 Pi = Position.SampleLevel(Sampler, floatingCoords + h * float2(cos(theta), sin(theta)), 0);
        float3 wi = Pi - P;
		float di = length(eyePos - Pi.xyz);
		float H = 1.0f;
		
		if (R - length(wi) < 0.0f)
			H = 0.0f;

		sum += max(0.0, dot(N,wi) - delta * di) * H / max(c*c, dot(wi,wi));
	}

	sum *= 2 * PI * c/n;
	float result = pow((1.0 - s * sum), k);

    return float4(float3(result, result, result), 1.0f);
}