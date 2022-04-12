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

float3 calculateWavePosition(float3 position)
{
	float Q = 0.8f;
	const int numWaves = 3;

	const WaveParameters _WaveParameters[numWaves] = {
		{2, 0.2f, 0.001f, float2(0.2, 1)},
		{0.5, 0.5f, 0.002f, float2(0.4, 1)},
		{0.2f, 1.2f, 0.004f, float2(-0.2, -1)}
	};

	float yOffset = 0;
	for (int i = 0; i < numWaves; ++i)
	{
		const WaveParameters waveParameters = _WaveParameters[i];
		yOffset += waveParameters.amplitude * sin(waveParameters.frequency * dot(waveParameters.direction, position.xz) + time * waveParameters.speed);
	}

	float2 xzOffset = 0;
	for (int i = 0; i < numWaves; ++i)
	{
		const WaveParameters waveParameters = _WaveParameters[i];
		float q = Q / (waveParameters.frequency * waveParameters.amplitude * numWaves);

		xzOffset += q * waveParameters.amplitude * waveParameters.direction * cos(waveParameters.frequency * dot(waveParameters.direction, position.xz) + time * waveParameters.speed);
	}

	return float3(position.x + xzOffset.x, position.y + yOffset, position.z + xzOffset.y);
}

float3 calculateWaveNormal(float3 position)
{
	float Q = 0.8f;
	const int numWaves = 3;

	const WaveParameters _WaveParameters[numWaves] = {
		{2, 0.2f, 0.001f, float2(0.2, 1)},
		{0.5, 0.5f, 0.002f, float2(0.4, 1)},
		{0.2f, 1.2f, 0.004f, float2(-0.2, -1)}
	};

	float y = 0;
	for (int i = 0; i < numWaves; ++i)
	{
		const WaveParameters waveParameters = _WaveParameters[i];
		float q = Q / (waveParameters.frequency * waveParameters.amplitude * numWaves);
		y += q * waveParameters.frequency * waveParameters.amplitude * sin(waveParameters.frequency * dot(waveParameters.direction, position.xz) + time * waveParameters.speed);
	}

	float2 xz = 0;
	for (int i = 0; i < numWaves; ++i)
	{
		const WaveParameters waveParameters = _WaveParameters[i];
		xz += waveParameters.direction * waveParameters.frequency * waveParameters.amplitude * cos(waveParameters.frequency * dot(waveParameters.direction, position.xz) + time * waveParameters.speed);
	}

	return float3(-xz.x, 1 - y, -xz.y);
}

float3 calculateWaveTangent(float3 position)
{
	float Q = 0.8f;
	const int numWaves = 3;

	const WaveParameters _WaveParameters[numWaves] = {
		{2, 0.2f, 0.001f, float2(0.2, 1)},
		{0.5, 0.5f, 0.002f, float2(0.4, 1)},
		{0.2f, 1.2f, 0.004f, float2(-0.2, -1)}
	};

	float x = 0;
	for (int i = 0; i < numWaves; ++i)
	{
		const WaveParameters waveParameters = _WaveParameters[i];
		float q = Q / (waveParameters.frequency * waveParameters.amplitude * numWaves);
		x += q * waveParameters.direction.x * waveParameters.direction.y * waveParameters.frequency * waveParameters.amplitude * 
			sin(waveParameters.frequency * dot(waveParameters.direction, position.xz) + time * waveParameters.speed);
	}

	float y = 0;
	for (int i = 0; i < numWaves; ++i)
	{
		const WaveParameters waveParameters = _WaveParameters[i];
		y += waveParameters.direction.y * waveParameters.frequency * waveParameters.amplitude *
			cos(waveParameters.frequency * dot(waveParameters.direction, position.xz) + time * waveParameters.speed);
	}

	float z = 0;
	for (int i = 0; i < numWaves; ++i)
	{
		const WaveParameters waveParameters = _WaveParameters[i];
		float q = Q / (waveParameters.frequency * waveParameters.amplitude * numWaves);
		z += q * waveParameters.direction.y * waveParameters.direction.y * waveParameters.frequency * waveParameters.amplitude *
			sin(waveParameters.frequency * dot(waveParameters.direction, position.xz) + time * waveParameters.speed);
	}
	return float3(-x, y, 1 - z);
}

struct VS_OUTPUT
{
	float4 Position : SV_Position;
	float4 WorldPosition : Position;
	float3 Normal : Normal;
	float3 Tangent : Tangent;
	float3 BiTangent : Bitangent;
};

VS_OUTPUT main(float3 pos : Position)
{
	VS_OUTPUT output;

	float4 modelPosition = mul(float4(pos, 1.0f), model);

	float3 newPosition = calculateWavePosition(modelPosition.xyz);
	
	output.Position = mul(mul(float4(newPosition, 1.0f), view), projection);
	output.WorldPosition = float4(newPosition, 1.0f);
	output.Normal = calculateWaveNormal(modelPosition.xyz);
	output.Tangent = calculateWaveTangent(modelPosition.xyz);
	output.BiTangent = cross(output.Tangent, output.Normal);

	return output;
}