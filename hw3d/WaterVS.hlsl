#include "ShaderOps.hlsl"

cbuffer TimeCBuf
{
	float time;
	matrix modelView;
	matrix modelViewProjection;
	matrix model;
	matrix view;
	matrix projection;
};


struct VS_OUTPUT
{
	float4 Position : SV_Position;
	float4 WorldPosition : Position;
	float3 Normal : Normal;
	float3 Tangent : Tangent;
	float3 BiTangent : Bitangent;
	float2 tc : Texcoord;
};

VS_OUTPUT main(float3 pos : Position, float2 tc : Texcoord)
{
	VS_OUTPUT output;

	float4 modelPosition = mul(float4(pos, 1.0f), model);

	WaterOutput waterOutput = calculateWaterOutput(modelPosition, time);
	
	output.Position = mul(mul(float4(waterOutput.Position, 1.0f), view), projection);
	output.WorldPosition = float4(waterOutput.Position, 1.0f);
	output.Normal = waterOutput.Normal;
	output.Tangent = waterOutput.Tangent;
	output.BiTangent = waterOutput.BiTangent;
	output.tc = tc;

	return output;
}