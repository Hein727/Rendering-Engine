#include "ImageBasedLighting.hlsli"

#ifndef _BIDIRECTIONALREFLECTANCEDISTRIBUTIONFUNCTION_HLSL_
#define _BIDIRECTIONALREFLECTANCEDISTRIBUTIONFUNCTION_HLSL_

float3 FSchlick(float3 f0, float3 f90, float VoH)
{
    return f0 + (f90 - f0) * pow(clamp(1.0 - VoH, 0.0, 1.0), 5.0);
}

float Vggx(float NoL, float NoV, float alphaRoughness)
{
    float alpahRoughnessSq = alphaRoughness * alphaRoughness;
    
    float ggxv = NoL * sqrt(NoV * NoV * (1.0f - alpahRoughnessSq) + alpahRoughnessSq);
    float ggxl = NoV * sqrt(NoL * NoL * (1.0f - alpahRoughnessSq) + alpahRoughnessSq);
    
    float ggx = ggxv + ggxl;
    return (ggx > 0.0f) ? 0.5f / ggx : 0.0f;    
}

float Dggx(float NoH, float alphaRoughness)
{
    const float PI = 3.14159265358979f;
    float alphaRoughnessSq = alphaRoughness * alphaRoughness;
    float f = (NoH * NoH) * (alphaRoughnessSq - 1.0f) + 1.0f;
    return alphaRoughnessSq / (PI * f * f);
}

float3 BrdfLambertian(float3 f0, float3 f90, float3 diffuseColor, float VoH)
{
    const float PI = 3.14159265358979f;
    return (1.0f - FSchlick(f0, f90, VoH)) * (diffuseColor / PI);
}

float3 BrdfSpecularGgx(float3 f0, float3 f90, float alphaRoughness, float VoH, float NoL, float NoV, float NoH)
{
    float3 F = FSchlick(f0, f90, VoH);
    float Vis = Vggx(NoL, NoV, alphaRoughness);
    float D = Dggx(NoH, alphaRoughness);
    
    return F * Vis * D;
}

float3 IblRadianceLambertian(float3 N, float3 V, float roughness, float3 diffuseColor, float3 f0)
{
    float NoV = clamp(dot(N, V), 0.0, 1.0);
    
    float2 brdfSamplePoint = clamp(float2(NoV, roughness), 0.0, 1.0);
    float2 fAb = SampleLutGGX(brdfSamplePoint).rg;
    
    float3 irradiance = SampleDiffuseIem(N).rgb;
    
    float3 fr = max(1.0 - roughness, f0) - f0;
    float3 kS = f0 + fr * pow(1.0 - NoV, 5.0);
    float3 fssEss = kS * fAb.x + fAb.y;
    
    float ems = (1.0f - (fAb.x + fAb.y));
    float3 fAvg = (f0 + (1.0 - f0) / 21.0);
    float3 fmsEms = ems * fssEss * fAvg / (1.0 - fAvg * ems);
    float3 kD = diffuseColor * (1.0 - fssEss + fmsEms);
    
    return (fmsEms + kD) * irradiance;
}


// problem ?
float3 IblRadianceGGX(float3 N, float3 V, float roughness, float3 f0)
{
    float NoV = clamp(dot(N, V), 0.0, 1.0);
    
    float2 brdfSamplePoint = clamp(float2(NoV, roughness), 0.0, 1.0);
    float2 fAb = SampleLutGGX(brdfSamplePoint).rg;
    
    float3 R = normalize(reflect(-V, N));
    float3 specularLight = SampleSpecularPmrem(R, roughness).rgb;
    
    float3 fr = max(1.0 - roughness, f0) - f0;
    float3 kS = f0 + fr * pow(1.0 - NoV, 5.0);
    float3 fssEss = kS * fAb.x + fAb.y;
    
    return specularLight * fssEss;
}
#endif 
