#include "LineRenderer.hlsli"

VS_OUT main(VS_IN input)
{
    VS_OUT output;
    output.position = mul(input.position, view_projection);
    output.color = input.color;
    return output;
}