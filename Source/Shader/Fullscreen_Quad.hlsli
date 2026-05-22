struct VS_OUT
{
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD;
};

cbuffer LuminanceParams : register(b1)
{
    float luminanceThreshold;
    float3 padding;
};

cbuffer BlurParams : register(b2)
{
    float gs; // Gaussian blur sigma
    float bi; // Blur iteration
    float expo; // Exponential blur power
    float dummy;
}