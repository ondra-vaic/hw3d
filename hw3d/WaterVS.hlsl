
cbuffer TimeCBuf
{
	float time;
	matrix modelView;
	matrix modelViewProjection;
	matrix model;
	matrix view;
	matrix projection;
};

struct WaveParameters
{
	float amplitude;
	float frequency;
	float speed;
	float2 direction;
};

float getHeight(float2 position, float amplitude, float frequency, float speed, float2 direction)
{
	return amplitude * sin(dot(direction * frequency, position) + time * speed);
}

float3 getGerstnerWavePosition(float3 position)
{
	float Q = 0.8f;
	const int numWaves = 3;

	const WaveParameters _WaveParameters[numWaves] = {
		{2, 0.2f, 0.001f, float2(-0.2f, 1)},
		{0.5, 0.5f, 0.002f, float2(0.2f, 1)},
		{0.1f, 1.2f, 0.004f, float2(-0.4f, 1)}
	};

	float yOffset = 0;

	for (int i = 0; i < numWaves; ++i)
	{
		const WaveParameters waveParameters = _WaveParameters[i];
		yOffset += getHeight(position.xz, waveParameters.amplitude, waveParameters.frequency, waveParameters.speed, waveParameters.direction);
	}

	float2 xzOffset = 0;

	for (int i = 0; i < numWaves; ++i)
	{
		const WaveParameters waveParameters = _WaveParameters[i];
		float q = Q / (waveParameters.frequency * waveParameters.amplitude * numWaves);

		xzOffset += q * waveParameters.amplitude * waveParameters.direction * cos(dot(waveParameters.frequency * waveParameters.direction, position.xz) + time * waveParameters.speed);
	}

	return float3(position.x + xzOffset.x, position.y + yOffset, position.z + xzOffset.y);
}

float4 main(float3 pos : Position) : SV_Position
{
	float4 modelPosition = mul(float4(pos, 1.0f), model);

	float3 newPosition = getGerstnerWavePosition(modelPosition.xyz);
	return mul(mul(float4(newPosition, 1.0f), view), projection);
}