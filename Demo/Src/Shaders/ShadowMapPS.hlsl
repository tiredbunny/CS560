
struct VertexOut
{
	float4 PosH : SV_POSITION;
};

float4 main(VertexOut pin) : SV_TARGET
{
	float z = pin.PosH.z;

	return float4(z, z*z, z*z*z, z*z*z*z);

}