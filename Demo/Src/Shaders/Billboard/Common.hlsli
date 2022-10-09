struct VertexIn
{
    float3 CenterW : CENTER;
    float2 Size : SIZE;
};

typedef VertexIn VertexOut;

struct GSOut
{
    float4 PosH : SV_Position;
    float2 TexCoord : TEXCOORD;
    uint PrimID : SV_PrimitiveID;
};