
cbuffer TimeCBuf
{
	float time;
	matrix modelView;
	matrix modelViewProjection;
	matrix model;
	matrix view;
	matrix projection;
};

float4 main(float3 pos : Position) : SV_Position
{
	float3 direction = float3(0, 0, 0);

	float4 modelPosition = mul(float4(pos, 1.0f), model);

	float3 newPosition = float3(modelPosition.x, modelPosition.y + sin(modelPosition.z * 0.5f + time * 0.01f) * 5, modelPosition.z);
	return mul(mul(float4(newPosition, 1.0f), view), projection);
}