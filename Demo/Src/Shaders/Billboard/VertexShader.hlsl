#include "Common.hlsli"

VertexOut main( VertexIn vin )
{
    VertexOut vout;
    
    vout.CenterW = vin.CenterW;
    vout.Size = vin.Size;
    
    return vout;
}