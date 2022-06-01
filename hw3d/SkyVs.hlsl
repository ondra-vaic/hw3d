#include "Transform.hlsl"

struct VS_OUTPUT
{
	float4 Position : SV_Position;
	float3 WorldPosition : Position;
};

VS_OUTPUT main(float3 pos : Position)
{
	VS_OUTPUT output;

	output.Position = mul(float4(pos, 1.0f), modelViewProj);
	output.WorldPosition = pos;

	return output;
}