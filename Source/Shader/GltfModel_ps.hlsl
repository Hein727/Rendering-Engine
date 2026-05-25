#include "GltfModel.hlsli"

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

float4 main(VS_OUT pin) : SV_TARGET
{
    MaterialConstants m = materials[material];
    
    float3 N = normalize(pin.w_normal.xyz);
    float3 L = normalize(-light_direction.xyz);
    
    float3 color = max(0, dot(N, L)) * m.pbrMetallicRoughness.basecolorFactor.rgb;

    return float4(color, 1);
}