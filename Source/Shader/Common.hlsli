#define LINEAR_WRAP 0
#define LINEAR_CLAMP 1
#define ANISOTROPIC_WRAP 2
#define POINT_CLAMP 3
#define POINT_WRAP 4
#define SHADOW_COMPARISON_SAMPLER 5
#define NORMAL_MAP_SAMPLER 6

SamplerState sampler_states[7] : register(s0);

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

// DO NOT USE b0 or b13 TO ASSIGN ANOTHER CBUFFER! It will conflict with SCENE_CONSTANTS!