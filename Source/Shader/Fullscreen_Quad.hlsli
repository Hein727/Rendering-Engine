struct VS_OUT
{
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD;
};

cbuffer LuminanceParams : register(b1)
{
    float luminanceThreshold;
};

cbuffer BlurParams : register(b2)
{
    float gs;
    float bi;
    float expo;
}