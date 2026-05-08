struct VS_OUT
{
    float4 position : SV_Position;
    float4 color : COLOR;
};

cbuffer OBJECT_CONSTANTS : register(b1)
{
    row_major float4x4 world;
    float4 material_color;
}

cbuffer SCENE_CONSTANTS : register(b2)
{
    row_major float4x4 view_projection;
    float4 light_direction;
}