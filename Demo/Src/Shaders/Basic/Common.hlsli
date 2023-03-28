#include "..\Lighting.hlsli"

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : WORLD_POS;
    float3 NormalW : NORMAL0;
    float2 Tex : TEXCOORD0;
    float4 ShadowPosH : TEXCOORD1;
};


cbuffer cbPerObject : register(b0)
{
    row_major float4x4 gWorldViewProj;
    row_major float4x4 gWorld;
    row_major float4x4 gWorldInverseTranspose;
    row_major float4x4 gTextureTransform;
    row_major float4x4 gShadowTransform;

    Material gMaterial;
};

float VarianceMethod(float4 moments, float rel_pixel_depth) 
{
    float rel_light_depth = moments.x;

    float res = 1.0f;
    if (rel_pixel_depth > rel_light_depth) 
    {
        float mean = moments.x;
        float variance = moments.y - pow(mean, 2);

        res = variance / (variance + pow(rel_pixel_depth - mean, 2));
    }

    return res;
}

float3 CholeskyDecomposition(float4 b_dash, float rel_pixel_depth)
{
    float m11 = 1.0f;
    float m12 = b_dash.x;
    float m13 = b_dash.y;
    float m22 = b_dash.y;
    float m23 = b_dash.z;
    float m33 = b_dash.w;

    float z1 = 1.0f;
    float z2 = rel_pixel_depth;
    float z3 = rel_pixel_depth * rel_pixel_depth;

    float a = 1.0f; //sqrt(m11);
//    if (a <= 0.0f) {
//        a = 0.0001f;
//    }

    float b = m12 / a;
    float c = m13 / a;

    float d = sqrt(m22 - (b * b));
    if (d <= 0.0f)
    {
        d = 0.0001f;
    }

    float e = (m23 - (b * c)) / d;
    float f = sqrt(m33 - (c * c) - (e * e));

    if (f <= 0.0f)
    {
        f = 0.0001f;
    }

    float c1_hat = z1 / a;
    float c2_hat = (z2 - (b * c1_hat)) / d;
    float c3_hat = (z3 - (c * c1_hat) - (e * c2_hat)) / f;

    float c3 = c3_hat / f;
    float c2 = (c2_hat - (e * c3)) / d;
    float c1 = (c1_hat - (b * c2) - (c * c3)) / a;

    return float3(c1, c2, c3);
}

float Hamburger4MSMMethod(float4 moments, float rel_pixel_depth)
{
    float bias = 0.0001f;
    float4 b_dash = ((1.0f - bias) * moments) + (bias * float4(0.5f, 0.5f, 0.5f, 0.5f));

    // c = (c1, c2, c3)
    float3 c = CholeskyDecomposition(b_dash, rel_pixel_depth);

    // in quadratic equation, a = c3, b = c2, c = c1
    float sqrt_det = sqrt((c.y * c.y) - (4.0f * c.z * c.x));

    // roots = z2, z3
    float z2 = 0.0f;
    float z3 = 0.0f;

    z2 = (-c.y - sqrt_det) / (2.0f * c.z);
    // z2 should always be the smaller root
    if (c.z < 0.0f) 
    {
        z3 = z2;
        z2 = (-c.y + sqrt_det) / (2.0f * c.z);
    }
    else {
        z3 = (-c.y + sqrt_det) / (2.0f * c.z);
    }

    float G = 0.0f;
    if (rel_pixel_depth <= z2) 
    {
        G = 0.0f;
    }
    else if (rel_pixel_depth <= z3) 
    {
        G = ((rel_pixel_depth * z3) - (b_dash.x * (rel_pixel_depth + z3)) + (b_dash.y))
            / ((z3 - z2) * (rel_pixel_depth - z2));
    }
    else {
        G = 1 -
            (((z2 * z3) - (b_dash.x * (z2 + z3)) + (b_dash.y))
                / ((rel_pixel_depth - z2) * (rel_pixel_depth - z3)));
    }

    return G;
}

