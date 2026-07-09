struct DirectionalLight
{
    float4 directional;
    float4 color;
};

struct PointLight
{
    float4 position;
    float4 color;
    float range;
    float3 padding;
};

struct SpotLight
{
    float4 position;
    float4 direction;
    float4 color;
    float range;
    float innerCone;
    float outerCone;
    float3 padding;
};

struct HemisphereLight
{
    float4 skyColor;
    float4 groundColor;
    float weight;
    float3 padding;
};