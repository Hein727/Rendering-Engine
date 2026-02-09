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
    float4 light_direction;
    float4 camera_position;
}

// DO NOT USE b0 TO ASSIGN ANOTHER CBUFFER! It will conflict with SCENE_CONSTANTS!