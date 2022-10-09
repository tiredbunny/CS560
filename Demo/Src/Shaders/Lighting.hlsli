
struct Material
{
    float4 Ambient; 
    float4 Diffuse;
    float4 Specular;
};

struct DirectionalLight
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float3 Direction;
    float pad;
};

struct PointLight
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    
    float3 Position;
    float Range;
    
    float3 Attenuation;
    float pad;
};

struct SpotLight
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    
    float3 Direction;
    float SpotPower;
    
    float3 Position;
    float Range;
    
    float3 Attenuation;
    float pad;
};

struct FogProperties
{
    float FogStart;
    float FogRange;
    float FogEnabled; //set to <= 0.0f to disable
    float pad;
    float4 FogColor;
};


typedef Material LightingOutput;

LightingOutput ComputeDirectionalLight(Material mat, DirectionalLight light, float3 normal, float3 toEyeVector)
{
    LightingOutput result;
    
    float diffuseIntensity = saturate(dot(-light.Direction, normal));
    
    float specularIntensity = 0.0f;
    [flatten]
    if (diffuseIntensity > 0.0f)
    {
        float3 reflectVector = normalize(reflect(light.Direction, normal));
        specularIntensity = pow(saturate(dot(reflectVector, toEyeVector)), mat.Specular.w);
    }
    
    result.Ambient = mat.Ambient * light.Ambient;
    result.Diffuse = diffuseIntensity * (mat.Diffuse * light.Diffuse);
    result.Specular = specularIntensity * (mat.Specular * light.Specular);

    return result;
}


LightingOutput ComputePointLight(Material mat, PointLight light, float3 normal, float3 pixelPos, float3 toEyeVector)
{
    float3 surfaceToLightVec = light.Position - pixelPos;
    float vectorLength = length(surfaceToLightVec);
    
    LightingOutput result;
    
    result.Ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    result.Diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    result.Specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    if (vectorLength > light.Range)
        return result;
    
    //normalize the vector ourselves since we already computed length
    surfaceToLightVec /= vectorLength;
    
    float diffuseIntensity = saturate(dot(surfaceToLightVec, normal));
    
    float specularIntensity = 0.0f;
    [flatten]
    if (diffuseIntensity > 0.0f)
    {
        float3 reflectVector = reflect(-surfaceToLightVec, normal);
        specularIntensity = pow(saturate(dot(reflectVector, toEyeVector)), mat.Specular.w);
    }
    
    //attenuation equation
    //  1 / ( a0 + a1*d + a2* d^2 )
    float att = 1.0f / dot(light.Attenuation, float3(1.0f, vectorLength, vectorLength * vectorLength));

    result.Diffuse = diffuseIntensity * (mat.Diffuse * light.Diffuse) * att;
    result.Specular = specularIntensity * (mat.Specular * light.Specular) * att;
    result.Ambient = mat.Ambient * light.Ambient;
    
    return result;
}

LightingOutput ComputeSpotLight(Material mat, SpotLight light, float3 normal, float3 pixelPos, float3 toEyeVector)
{
    float3 surfaceToLightVec = light.Position - pixelPos;
    float vectorLength = length(surfaceToLightVec);
    
    LightingOutput result;
    
    result.Ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    result.Diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    result.Specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    if (vectorLength > light.Range)
        return result;
    
    //normalize the vector ourselves since we already computed length
    surfaceToLightVec /= vectorLength;
    
    float diffuseIntensity = saturate((dot(surfaceToLightVec, normal)));
    
    float specularIntensity = 0.0f;
    [flatten]
    if (diffuseIntensity > 0.0f)
    {
        float3 reflectVector = reflect(-surfaceToLightVec, normal);
        specularIntensity = pow(saturate(dot(reflectVector, toEyeVector)), mat.Specular.w);
    }
    
    float coneSpotIntensity = pow(saturate(dot(-light.Direction, surfaceToLightVec)), light.SpotPower);
    //attenuation equation
    // 1 / ( a0 + a1*d + a2* d^2 )
    float att = 1.0f / dot(light.Attenuation, float3(1.0f, vectorLength, vectorLength * vectorLength));
    
    result.Diffuse = diffuseIntensity * (mat.Diffuse * light.Diffuse) * att * coneSpotIntensity;
    result.Specular = specularIntensity * (mat.Specular * light.Specular) * att * coneSpotIntensity;
    result.Ambient = (mat.Ambient * light.Ambient) * coneSpotIntensity;
    
    return result;
}