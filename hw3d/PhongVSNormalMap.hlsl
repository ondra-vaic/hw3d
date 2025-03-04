#include "Transform.hlsl"

struct VSOut
{
    float4 pos : SV_Position;
    float4 worldPosition : Color;
    float4 worldNormal : Color1;
    float3 viewPos : Position;
    float3 viewNormal : Normal;
    float3 tan : Tangent;
    float3 bitan : Bitangent;
    float2 tc : Texcoord;
};

VSOut main(float3 pos : Position, float3 n : Normal, float3 tan : Tangent, float3 bitan : Bitangent, float2 tc : Texcoord)
{
    VSOut vso;

    vso.worldNormal = mul(float4(n, 0.0f), model);
    vso.viewPos = (float3) mul(float4(pos, 1.0f), modelView);
    vso.worldPosition = mul(float4(pos, 1.0f), model);
    vso.viewNormal = mul(n, (float3x3) modelView);
    vso.tan = mul(tan, (float3x3) modelView);
    vso.bitan = mul(bitan, (float3x3) modelView);    
    vso.pos = mul(float4(pos, 1.0f), modelViewProj);
    vso.tc = tc;
    return vso;
}