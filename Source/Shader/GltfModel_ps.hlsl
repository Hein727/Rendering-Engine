#include "GltfModel.hlsli"
#include "BidirectionalReflectanceDistributionFunction.hlsli"
#include "ShaderingFunctions.hlsli"

struct TextureInfo
{
    int index;
    int texcoord;
};
struct NormalTextureInfo
{
    int index;
    int texcoord;
    float scale;
};
struct OcclusionTextureInfo
{
    int index;
    int texcoord;
    float strength;
};
struct PbrMetallicRoughness
{
    float4 basecolorFactor;
    TextureInfo basecolorTexture;
    float metallicFactor;
    float roughnessFactor;
    TextureInfo metallicRoughnessTexture;
};
struct MaterialConstants
{
    float3 emissiveFactor;
    int alphaMode;
    float alphaCutOff;
    int doubleSided;
    
    PbrMetallicRoughness pbrMetallicRoughness;
    
    NormalTextureInfo normalTexture;
    OcclusionTextureInfo occlusionTexture;
    TextureInfo emissiveTexture;
};
StructuredBuffer<MaterialConstants> materials : register(t0);

#define BASECOLOR_TEXTURE 0
#define METALLIC_ROUGHNESS_TEXTURE 1
#define NORMAL_TEXTURE 2
#define EMISSIVE_TEXTURE 3
#define OCCLUSION_TEXTURE 4
Texture2D<float4> materialTextures[5] : register(t1);  
Texture2D<float4> shadowMapTexture : register(t126); 
TextureCube environmentMapTexture : register(t127); 

// SamplerStates are already defined in Common.hlsli, so we can just use them here.

float4 main(VS_OUT pin) : SV_TARGET
{
    const float GAMMA = 2.2f;
    
    const MaterialConstants m = materials[material];
    
    float4 baseColorFactor = m.pbrMetallicRoughness.basecolorFactor;
    const int baseColorTextureIndex = m.pbrMetallicRoughness.basecolorTexture.index;
    
    if(baseColorTextureIndex > -1)
    {
        float4 sampled = materialTextures[BASECOLOR_TEXTURE].Sample(sampler_states[ANISOTROPIC_WRAP], pin.texcoord);
        sampled.rgb = pow(sampled.rgb, GAMMA);
        baseColorFactor *= sampled;
    }

    float3 emissiveFactor = m.emissiveFactor;
    const int emissiveTextureIndex = m.emissiveTexture.index;   
    if(emissiveTextureIndex > -1)
    {
        float4 sampled = materialTextures[EMISSIVE_TEXTURE].Sample(sampler_states[ANISOTROPIC_WRAP], pin.texcoord);
        sampled.rgb = pow(sampled.rgb, GAMMA);
        emissiveFactor *= sampled.rgb;
    }
    
    float roughnessFactor = m.pbrMetallicRoughness.roughnessFactor;
    float metallicFactor = m.pbrMetallicRoughness.metallicFactor;
    const int metallicRoughnessTextureIndex = m.pbrMetallicRoughness.metallicRoughnessTexture.index;
    if(metallicRoughnessTextureIndex > -1)
    {
        float4 sampled = materialTextures[METALLIC_ROUGHNESS_TEXTURE].Sample(sampler_states[LINEAR_WRAP], pin.texcoord);
        roughnessFactor *= sampled.g;
        metallicFactor *= sampled.b;
    }
    
    float occlusionFactor = 1.0f;
    const int occlusionTextureIndex = m.occlusionTexture.index;
    if(occlusionTextureIndex > -1)
    {
        float4 sampled = materialTextures[OCCLUSION_TEXTURE].Sample(sampler_states[LINEAR_WRAP], pin.texcoord);
        occlusionFactor *= sampled.r;
    }
    const float occlusionStrength = m.occlusionTexture.strength;    
    
    const float3 f0 = lerp(0.04, baseColorFactor.rgb, metallicFactor);
    const float3 f90 = 1.0;
    const float alphaRoughness = roughnessFactor * roughnessFactor;
    const float3 CDiff = lerp(baseColorFactor.rgb, 0.0f, metallicFactor);
    
    const float3 P = pin.w_position.xyz;
    const float3 V = normalize(camera_position.xyz - pin.w_position.xyz);
    
    float3 N = normalize(pin.w_normal.xyz);
    float3 T = hasTangent ? normalize(pin.w_tangent.xyz) : float3(1, 0, 0);
    float sigma = hasTangent ? pin.w_tangent.w : 1.0f;
    T = normalize(T - N * dot(N, T));
    float3 B = normalize(cross(N, T) * sigma);
    
    const int normalTexture = m.normalTexture.index;
    if(normalTexture > -1)
    {
        float4 sampled = materialTextures[NORMAL_TEXTURE].Sample(sampler_states[LINEAR_WRAP], pin.texcoord);
        float3 normalFactor = sampled.xyz;
        normalFactor = (normalFactor * 2.0f) - 1.0f;
        normalFactor = normalize(normalFactor * float3(m.normalTexture.scale, m.normalTexture.scale, 1.0));
        N = normalize((normalFactor.x * T) + (normalFactor.y * B) + (normalFactor.z * N));
    }
    
    float3 diffuse = 0;
    float3 specular = 0;
    
    float4 color = (float4) 0;
    {
        float3 ambient = ambientColor.rgb * ambientColor.a;
        
        float3 directionalDiuffse = 0, directionalSpecular = 0;
        {
            float3 L = normalize(directionalLight.directional.xyz);
            float3 LC = directionalLight.color.rgb * directionalLight.color.a;
            directionalDiuffse = CalcLambert(N, L, LC, 1);
            directionalSpecular = CalcPhongSpecular(N, V, L, LC, 1);
            
            float depth = shadowMapTexture.SampleCmpLevelZero(shadow_samplers[SHADOW_COMPARISON_SAMPLER], pin.shadow_texcoord.xy, pin.shadow_texcoord.z - shadow_bias).r;
            
            float3 shadowFactor = lerp(shadow_color, float3(1, 1, 1), depth);
            
            directionalDiuffse *= shadowFactor;
            directionalSpecular *= shadowFactor;
        }
        
        float3 pointDiffuse = 0, pointSpecular = 0;
        for (int i = 0; i < MAX_DATA; ++i)
        {
            if (i >= lightCount.y)
                break;
            
            float3 L = pin.w_position.xyz - pointLight[i].position.xyz;
            float len = length(L);
            if(len >= pointLight[i].range)
                continue;
            float attenuateLength = saturate(1.0f - len / pointLight[i].range);
            float attenuation = attenuateLength * attenuateLength;
            L /= len;
            float3 LC = pointLight[i].color.rgb * pointLight[i].color.a;
            pointDiffuse += CalcLambert(N, L, LC, 1) * attenuation;
            pointSpecular += CalcPhongSpecular(N, V, L, LC, 1) * attenuation; 
        }
        
        float3 spotDiffuse = 0, spotSpecular = 0;
        for (int j = 0; j < 8; ++j)
        {
            if(j >= lightCount.z)
                break;
            
            float3 L = pin.w_position.xyz - spotLight[j].position.xyz;
            float len = length(L);
            if(len >= spotLight[j].range)
                continue;
            float attenuateLength = saturate(1.0f - len / spotLight[j].range);
            float attenuation = attenuateLength * attenuateLength;
            L /= len;
            float3 spotDir = normalize(spotLight[j].direction.xyz);
            float angle = dot(spotDir, L);
            float area = spotLight[j].innerCone - spotLight[j].outerCone;
            attenuation *= saturate(1.0f - (spotLight[j].innerCone - angle) / area);
            float3 LC = spotLight[j].color.rgb * spotLight[j].color.a;
            spotDiffuse += CalcLambert(N, L, LC, 1) * attenuation;
            spotSpecular += CalcPhongSpecular(N, V, L, LC, 1) * attenuation;
        }

        color.a = baseColorFactor.a;
        color.rgb += baseColorFactor.rgb * (ambient + directionalDiuffse + pointDiffuse + spotDiffuse);
        color.rgb += directionalSpecular + spotSpecular + pointSpecular;
    }
    
    color.rgb += emissiveFactor;
    
     // Probe environment map for reflections    
    {
        float3 R = reflect(-V, N);
        float3 reflection = environmentMapTexture.Sample(sampler_states[LINEAR_WRAP], R).rgb;
        color.rgb += reflection * metallicFactor;
    }
    
    return color;
}
