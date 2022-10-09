#include "Common.hlsli"

cbuffer cbPerFrame : register(b0)
{
    float3 gEyePosW;
    float pad;
}

cbuffer cbPerObject : register(b1)
{
    row_major float4x4 gViewProj;
}

cbuffer cbFixed : register(b2)
{
    static const float2 gTexCoords[4] =
    {
        float2(0.0f, 1.0f),
        float2(0.0f, 0.0f),
        float2(1.0f, 1.0f),
        float2(1.0f, 0.0f)
    };
};

[maxvertexcount(4)]
void main(
	point VertexIn input[1],
	inout TriangleStream<GSOut> output,
	uint primID : SV_PrimitiveID)
{
    float3 upVec = float3(0.0f, 1.0f, 0.0f);
    
    float3 forwardVec = gEyePosW - input[0].CenterW;
    forwardVec.y = 0.0f;
    forwardVec = normalize(forwardVec);
    
    float3 rightVec = normalize(cross(upVec, forwardVec));
    
    float halfWidth = 0.5f * input[0].Size.x;
    float halfHeight = 0.5f * input[0].Size.y;
    
    float3 billboardVertex[4];
    billboardVertex[0] = input[0].CenterW + rightVec*halfWidth - upVec*halfHeight;
    billboardVertex[1] = input[0].CenterW + rightVec*halfWidth + upVec*halfHeight;
    billboardVertex[2] = input[0].CenterW - rightVec*halfWidth - upVec*halfHeight;
    billboardVertex[3] = input[0].CenterW - rightVec*halfWidth + upVec*halfHeight;
    
	[unroll]
    for (int i = 0; i < 4; ++i)
    {
        GSOut gout;
        gout.PosH = mul(float4(billboardVertex[i], 1.0f), gViewProj);
        gout.PrimID = primID;
        gout.TexCoord = gTexCoords[i];
        output.Append(gout);
    }

}