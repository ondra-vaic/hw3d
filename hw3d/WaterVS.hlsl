
cbuffer TimeCBuf
{
	float time;
	matrix modelView;
	matrix modelViewProjection;
	matrix model;
	matrix view;
	matrix projection;
};

float getHeight(float amplitude, float frequency, float speed, float power, float2 position, float2 direction)
{
	return amplitude * pow(sin(dot(direction, position) * frequency + time * speed), power);
}

float4 main(float3 pos : Position) : SV_Position
{
	float4 modelPosition = mul(float4(pos, 1.0f), model);

	const float wave1 = getHeight(3, 0.2f, 0.002f, 3, modelPosition.xz, float2(-0.2f, 1));
	const float wave2 = getHeight(1, 0.1f, 0.004f, 5, modelPosition.xz, float2(-0.2f, 1));
	const float wave3 = getHeight(0.35f, 0.03f, 0.008f, 8, modelPosition.xz, float2(-0.2f, 1));

	float height = wave1 + wave2 + wave3;

	float3 newPosition = float3(modelPosition.x, modelPosition.y + height, modelPosition.z);
	return mul(mul(float4(newPosition, 1.0f), view), projection);
}