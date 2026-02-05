#include "Static_Mesh.hlsli"
#include "Common.hlsli"

Texture2D color_map : register(t0);
Texture2D normal_map : register(t1);    
SamplerState point_sampler : register(s0);
SamplerState linear_sampler : register(s1);
SamplerState anisotropic_sampler : register(s2);

float4 main(VS_OUT pin) : SV_Target
{
    float4 color = color_map.Sample(anisotropic_sampler, pin.texcoord);
    float alpha = color.a;  
    float3 N = normalize(pin.world_normal.xyz);
    
    float3 T = float3(1.001, 0, 0);
    float3 B = normalize(cross(N, T));
    T = normalize(cross(B, N));
    
    float4 normal = normal_map.Sample(linear_sampler, pin.texcoord);
    normal = (normal * 2.0) - 1.0;
    normal.w = 0.0f;
    N = normalize((normal.x * T) + (normal.y * B) + (normal.z * N));
    
    float3 L = normalize(-light_direction.xyz);
    float3 diffuse = color.rgb * saturate(dot(N, L));
    
    float3 V = normalize(camera_position.xyz - pin.world_position.xyz);
    float3 specular = pow(saturate(dot(N, normalize(V + L))), 128);

    return float4(diffuse + specular, alpha) * pin.color;
}