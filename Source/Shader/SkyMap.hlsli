#include "Common.hlsli"

struct VS_OUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texcoord : TEXCOORD;
    float4 worldPosition : WORLD_POSITION;
};

cbuffer SKYMAP_CONSTANT_BUFFER : register(b1)
{
    row_major float4x4 inverese_view_projection;
};

#include "ShaderingFunctions.hlsli"