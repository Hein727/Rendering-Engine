cbuffer SCENE_CONSTANTS : register(b0)
{
    row_major float4x4 view_projection;
    float4 light_direction;
    float4 camera_position;
}