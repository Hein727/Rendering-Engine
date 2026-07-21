#include "SkyMap.hlsli"

Texture2D texture0 : register(t0);

float4 main(VS_OUT pin) : SV_TARGET
{
    float3 E = normalize(pin.worldPosition.xyz - camera_position.xyz);
    return SampleSkybox(texture0, sampler_states[LINEAR_WRAP], E);
}