struct VertexIn
{
	float4 PosH : SV_POSITION;
	float2 texCoord : TEXCOORD;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float2 texCoord : TEXCOORD;
};

VertexOut main(  VertexIn vin )
{
	VertexOut vout;

	vout.PosH = vin.PosH;
	vout.texCoord = vin.texCoord;

	return vout;
}