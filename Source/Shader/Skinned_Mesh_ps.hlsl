#include "Skinned_Mesh.hlsli"
#include "Common.hlsli"

Texture2D texture_maps[4] : register(t0); // Don't use t0 to t3 for other textures

float4 main(VS_OUT pin) : SV_Target
{
    float4 color = texture_maps[0].Sample(sampler_states[ANISOTROPIC_WRAP], pin.texcoord);
    float alpha = color.a;
    #if 1
    // Inverse gamma process
    const float GAMMA = 2.2;
    color.rgb = pow(color.rgb, GAMMA);
    #endif
    float3 N = normalize(pin.world_normal.xyz);
    float3 T = normalize(pin.world_tangent.xyz);
    float sigma = pin.world_tangent.w;
    T = normalize(T - N * dot(N, T));
    float3 B = normalize(cross(N, T) * sigma);
    
    float4 normal = texture_maps[1].Sample(sampler_states[LINEAR_WRAP], pin.texcoord);
    normal = (normal * 2.0f) - 1.0f;// Convert from [0, 1] to [-1, 1]
    N = normalize((normal.x * T) + (normal.y * B) + (normal.z * N));
    
    
    float3 L = normalize(-directionalLight.directional.xyz);
    float3 diffuse = color.rgb * max(0, dot(N, L));
    float3 V = normalize(camera_position.xyz - pin.world_position.xyz);
    float3 specular = pow(max(0, dot(N, normalize(V + L))), 128);
    return float4(diffuse + specular, alpha) * pin.color;
}