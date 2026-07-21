#define LINEAR_WRAP 0
#define LINEAR_CLAMP 1
#define ANISOTROPIC_WRAP 2
#define ANISOTROPIC_BORDER 3
#define POINT_CLAMP 4
#define POINT_WRAP 5
#define NORMAL_MAP_SAMPLER 6

#define SHADOW_COMPARISON_SAMPLER 0
#define SHADOW_BORDER_SAMPLER 1

SamplerState sampler_states[7] : register(s0);
SamplerComparisonState shadow_samplers[2] : register(s7);

cbuffer SCENE_CONSTANTS : register(b0)
{
    row_major float4x4 view_projection;
    float4 camera_position;
}

#include "Lights.hlsli"
#define MAX_DATA 8

cbuffer LIGHT_CONSTANTS : register(b13) 
{
    float4 ambientColor;
    uint4 lightCount;
    DirectionalLight directionalLight;
    PointLight pointLight[MAX_DATA];
    SpotLight spotLight[MAX_DATA];
    HemisphereLight hemisphereLight[MAX_DATA];
}

cbuffer SHADOW_CONSTANTS : register(b12)
{
    row_major float4x4 light_view_projection;
    float3 shadow_color;
    float shadow_bias;  
}

// DO NOT USE b0 or b13 TO ASSIGN ANOTHER CBUFFER! It will conflict with SCENE_CONSTANTS!
// Shadow Map is on b12 