#include "Fullscreen_Quad.hlsli"
#include "Common.hlsli"

Texture2D texture_maps[4] : register(t0);

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 color = texture_maps[0].Sample(sampler_states[ANISOTROPIC_WRAP], pin.texcoord);
    float alpha = color.a;
    color.rgb = step(luminanceThreshold, dot(color.rgb, float3(0.299, 0.587, 0.114))) * color.rgb;
    return float4(color.rgb, alpha);
}

