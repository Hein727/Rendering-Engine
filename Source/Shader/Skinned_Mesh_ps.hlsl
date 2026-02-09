#include "Skinned_Mesh.hlsli"
#include "Common.hlsli"

Texture2D texture_maps[4] : register(t0); // Don't use t0 to t3 for other textures

float4 main(VS_OUT pin) : SV_Target
{
    float4 color = texture_maps[0].Sample(sampler_states[ANISOTROPIC_WRAP], pin.texcoord);
    float3 N = normalize(pin.world_normal.xyz);
    float3 L = normalize(-light_direction.xyz);
    float3 diffuse = color.rgb * max(0, dot(N, L));
    return float4(diffuse, color.a) * pin.color;    
}