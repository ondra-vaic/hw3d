#include "Transform.hlsl"

struct VS_OUTPUT
{
	float4 pos : SV_Position;
	float4 worldPosition : Color;
	float4 worldNormal : Color1;
};

VS_OUTPUT main(float3 pos : Position)
{
	VS_OUTPUT output;

	output.pos = mul(float4(pos, 1.0f), modelViewProj);
	output.worldPosition = float4(pos, 1);
	output.worldNormal = float4(0, -1, 0, 0);

	return output;
}