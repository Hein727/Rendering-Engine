#include "GltfModel.hlsli"
#include "BidirectionalReflectanceDistributionFunction.hlsli"

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
    
    //metallicFactor = 0.0f; // For testing, we will set metallic to 0 to see the diffuse component clearly.
    
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
    
    // Loop for each light source
    float3 L = normalize(-light_direction.xyz);
    float3 Li = float3(1.0, 1.0, 1.0);
    const float NoL = max(0.0, dot(N, L));
    const float NoV = max(0.0, dot(N, V));
    if(NoL > 0.0 || NoV > 0.0)
    {
        const float3 R = reflect(-L, N);
        const float3 H = normalize(L + V);
        
        const float NoH = max(0.0, dot(N, H));
        const float HoV = max(0.0, dot(H, V));
        
        diffuse += Li * NoL * BrdfLambertian(f0, f90, CDiff, HoV);
        specular += Li * NoL * BrdfSpecularGgx(f0, f90, alphaRoughness, HoV, NoL, NoV, NoH);
    }
    
    diffuse += IblRadianceLambertian(N, V, roughnessFactor, CDiff, f0);
    specular += IblRadianceGGX(N, V, roughnessFactor, f0);
    
    float3 emissive = emissiveFactor;
    diffuse = lerp(diffuse, diffuse * occlusionFactor, occlusionStrength);
    specular = lerp(specular, specular * occlusionFactor, occlusionStrength);
    
    float3 Lo = diffuse + specular + emissive;
    
    return float4(Lo, baseColorFactor.a);
}
