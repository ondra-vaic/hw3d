#include "ShaderOps.hlsl"

cbuffer CBuf : register(b3)
{
	float3 LowColor;
	float3 HighColor;

	float3 CameraPosition;
}

float4 main(float4 Position : SV_Position, float3 worldPosition : Position) : SV_TARGET
{
	const float3 viewDir = normalize(worldPosition - CameraPosition);
	return SampleSkyBox(viewDir, LowColor, HighColor);
}