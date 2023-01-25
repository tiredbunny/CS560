struct VertexOut
{
	float4 screenSpace : SV_POSITION;
	float2 texCoord : TEXCOORD;
};

Texture2D Position	: register(t0);
Texture2D NormalMap		: register(t1);
Texture2D Texture	: register(t2);

float4 main(VertexOut pin) : SV_TARGET
{
	// gl_FragCoord.xy / vec2(width,height)

	
	return float4(0.0f, 1.0f, 0.0f, 1.0f);
}