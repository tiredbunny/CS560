cbuffer cbPerObject : register(b0)
{
    row_major float4x4 gWorldViewProj;
    row_major float4x4 gWorld;
    row_major float4x4 gWorldInverseTranspose;
    row_major float4x4 gTextureTransform;
    row_major float4x4 gShadowTransform;
};

struct VertexIn
{
    float3 PosL : SV_POSITION;
    float3 NormalL : NORMAL0;
    float2 Tex : TEXCOORD0;
};  

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : WORLD_POS;
    float3 NormalW : NORMAL0;
    float2 Tex : TEXCOORD0;
    float4 ShadowPosH : TEXCOORD1;
};

VertexOut main( VertexIn vin ) 
{ 
    VertexOut vout;
    
    vout.PosW = mul(float4(vin.PosL, 1.0f), gWorld).xyz;  
    vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
    vout.NormalW = mul(vin.NormalL, (float3x3) gWorldInverseTranspose);
    vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTextureTransform).xy;

    // Generate projective tex-coords to project shadow map onto scene.
    vout.ShadowPosH = mul(float4(vout.PosW, 1.0f), gShadowTransform);
    
    return vout;
}