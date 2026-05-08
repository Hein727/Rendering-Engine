#include "Sprite.hlsli"
#include "Common.hlsli"

Texture2D color_map : register(t0);

float4 main(VS_OUT pin) : SV_TARGET
{
    return color_map.Sample(sampler_states[POINT_CLAMP], pin.texcoord) * pin.color;
}