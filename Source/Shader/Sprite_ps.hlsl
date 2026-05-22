#include "Sprite.hlsli"
#include "Common.hlsli"

Texture2D color_map : register(t0);

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 color = color_map.Sample(sampler_states[ANISOTROPIC_WRAP], pin.texcoord);
    float alpha = color.a;
    #if 1
    // Inverse gamma process
    const float GAMMA = 2.2;
    color.rgb = pow(color.rgb, GAMMA);
    #endif
    return float4(color.rgb, alpha) * pin.color;
}