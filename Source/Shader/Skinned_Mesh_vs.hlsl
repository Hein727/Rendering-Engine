#include "Skinned_Mesh.hlsli"
#include "Common.hlsli"

VS_OUT main(VS_IN vin)
{
    VS_OUT vout;
    vin.normal.w = 0;
    float sigma = vin.tangent.w;
    vin.tangent.w = 0;
    
    float4 blended_position = float4(0, 0, 0, 1);
    float4 blended_normal = float4(0, 0, 0, 0);
    float4 blended_tangent = float4(0, 0, 0, 0);    
    
    for (int bone_index = 0; bone_index < 4; ++bone_index)
    {
        blended_position += vin.bone_weights[bone_index] * mul(vin.position, bone_transforms[vin.bone_indices[bone_index]]);
        blended_normal += vin.bone_weights[bone_index] * mul(vin.normal, bone_transforms[vin.bone_indices[bone_index]]);
        blended_tangent += vin.bone_weights[bone_index] * mul(vin.tangent, bone_transforms[vin.bone_indices[bone_index]]);  
    }
    
    vin.position = float4(blended_position.xyz, 1.0f);
    vin.normal = float4(blended_normal.xyz, 0.0f);   
    vin.tangent = float4(blended_tangent.xyz, 0.0f);
    
    vout.position = mul(vin.position, mul(world, view_projection));
    vout.world_position = mul(vin.position, world);
    vout.world_normal = normalize(mul(vin.normal, world));
    vout.world_tangent = normalize(mul(vin.tangent, world));
    vout.world_tangent.w = sigma;
    
    vout.texcoord = vin.texcoord;
    vout.color = material_color;
    
    return vout;
}