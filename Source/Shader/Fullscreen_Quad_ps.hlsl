#include "Common.hlsli"
#include "Fullscreen_Quad.hlsli"

Texture2D texture_map : register(t0);

float4 main(VS_OUT pin) : SV_TARGET
{
    return texture_map.Sample(sampler_states[LINEAR_WRAP], pin.texcoord);
}   