struct VertexOut
{
    float4 screenSpace : SV_POSITION;
    float2 TexC : TEXCOORD;
};

Texture2D gNormalMap : register(t0);
Texture2D Diffuse : register(t1);
Texture2D Position : register(t2);
Texture2D PBR : register(t3);

Texture2D gInputMap : register(t4);


static const int gBlurRadius = 5;

cbuffer cbPerFrame : register(b0)
{
    float4 gBlurWeights[3];
    
    float gHorizontalBlur;
    float width;
    float height;
    float preserveEdge;
    
    row_major float4x4 gProj;
}

float NdcDepthToViewDepth(float z_ndc)
{
    // z_ndc = A + B/viewZ, where gProj[2,2]=A and gProj[3,2]=B.
    float viewZ = gProj[3][2] / (z_ndc - gProj[2][2]);
    return viewZ;
}

float4 main(VertexOut pin) : SV_TARGET
{
	 // unpack into float array.
    float blurWeights[12] =
    {
        gBlurWeights[0].x, gBlurWeights[0].y, gBlurWeights[0].z, gBlurWeights[0].w,
        gBlurWeights[1].x, gBlurWeights[1].y, gBlurWeights[1].z, gBlurWeights[1].w,
        gBlurWeights[2].x, gBlurWeights[2].y, gBlurWeights[2].z, gBlurWeights[2].w,
    };
    
    int3 sampleIndex = int3(pin.screenSpace.xy, 0);
	
    float2 texOffset;
    if (gHorizontalBlur > 0.0f)
    {
        texOffset = float2(1.0f / width, 0.0f);
    }
    else
    {
        texOffset = float2(0.0f, 1.0f / height);
    }

    float4 color = blurWeights[gBlurRadius] * gInputMap.Load(sampleIndex);
    float totalWeight = blurWeights[gBlurRadius];
	 
    float3 centerNormal = normalize(gNormalMap.Load(sampleIndex).xyz);
    
    float centerDepth = NdcDepthToViewDepth(gNormalMap.Load(sampleIndex).w);

    for (float i = -gBlurRadius; i <= gBlurRadius; ++i)
    {
		// We already added in the center weight.
        if (i == 0)
            continue;

        float2 tex = pin.TexC + i * texOffset;
        
        int3 intCoords = int3(tex * float2(width, height), 0);
        
        float3 neighborNormal = normalize(gNormalMap.Load(intCoords).xyz);
        float neighborDepth = NdcDepthToViewDepth(gNormalMap.Load(intCoords).w);

        
        if (preserveEdge > 0.0f)
        {
            if (dot(neighborNormal, centerNormal) >= 0.8f && abs(neighborDepth - centerDepth) <= 0.2f)
            {
                float weight = blurWeights[i + gBlurRadius];

			    // Add neighbor pixel to blur.
                color += weight * gInputMap.Load(intCoords);
		
                totalWeight += weight;
            }
        }
        else
        {
            float weight = blurWeights[i + gBlurRadius];

			// Add neighbor pixel to blur.
            color += weight * gInputMap.Load(intCoords);
		
            totalWeight += weight;
        }
        
           
    }

	// Compensate for discarded samples by making total weights sum to 1.
    return color / totalWeight;
}