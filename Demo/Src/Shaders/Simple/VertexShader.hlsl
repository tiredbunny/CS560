#include "Common.hlsli"


float4 main( VertexIn vin ) : SV_Position
{
    return mul(float4(vin.PosL, 1.0f), gWorldViewProj);
}