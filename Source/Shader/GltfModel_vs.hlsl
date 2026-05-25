#include "GltfModel.hlsli"

VS_OUT main(VS_IN pin)
{
    VS_OUT vout;
    
    pin.position.w = 1;
    vout.position = mul(pin.position, mul(world, view_projection));
    vout.w_position = mul(pin.position, world);
    
    pin.normal.w = 0;
    vout.w_normal = normalize(mul(pin.normal, world));
    
    float sigma = pin.tangent.w;
    pin.tangent.w = 0;
    vout.w_tangent = normalize(mul(pin.tangent, world));
    vout.w_tangent.w = sigma;

    vout.texcoord = pin.texcoord;
    
    return vout;
}