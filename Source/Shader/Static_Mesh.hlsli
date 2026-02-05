struct VS_OUT
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 texcoord : TEXCOORD;
    float4 world_position : POSITION;  
    float4 world_normal : NORMAL;
};

cbuffer OBJECT_CONSTANTS : register(b1)
{
    row_major float4x4 world;
    float4 material_color;
}
