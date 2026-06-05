#include "Common.hlsli"
#ifndef _IMAGE_BASED_LIGHTING_
#define _IMAGE_BASED_LIGHTING_
    Texture2D skybox : register(t32);
    TextureCube diffuseIem : register(t33);
    TextureCube specularPmrem : register(t34);
    Texture2D lutGGX : register(t35);   

    float4 SampleLutGGX(float2 brdfSamplePoint)
    {
        return lutGGX.Sample(sampler_states[LINEAR_WRAP], brdfSamplePoint);
    }

    float4 SampleSkybox(float3 v, float roughness)
    {
        const float PI = 3.14159265358979f;
        uint width, height, numberOfLevels;
        skybox.GetDimensions(0,width, height, numberOfLevels);
    
        float lod = roughness * float(numberOfLevels - 1);
    
        v = normalize(v);
        // Blinn / Newell Latitude Mapping
        float2 samplePoint;
        samplePoint.x = (atan2(v.z, v.x) + PI) / (PI * 2.0);
        samplePoint.y = 1.0 - ((asin(v.y) + PI * 0.5) / PI);
        return skybox.SampleLevel(sampler_states[LINEAR_WRAP], samplePoint, lod);
    }

    float4 SampleDiffuseIem(float3 v)
    {
        return diffuseIem.Sample(sampler_states[LINEAR_WRAP], v);
    }

    float4 SampleSpecularPmrem(float3 v, float roughness)
    {
        uint width, height, numberOfLevels;
        specularPmrem.GetDimensions(0, width, height, numberOfLevels);
    
        float lod = roughness * float(numberOfLevels - 1);
   
        return specularPmrem.SampleLevel(sampler_states[LINEAR_WRAP], v, lod);
    }
#endif