float3 MapNormal(
    const in float3 tan,
    const in float3 bitan,
    const in float3 normal,
    const in float2 tc,
    uniform Texture2D nmap,
    uniform SamplerState splr)
{
    // build the tranform (rotation) into same space as tan/bitan/normal (target space)
    const float3x3 tanToTarget = float3x3(tan, bitan, normal);
    // sample and unpack the normal from normalTexture into target space   
    const float3 normalSample = nmap.Sample(splr, tc).xyz;
    const float3 tanNormal = normalSample * 2.0f - 1.0f;
    // bring normal from tanspace into target space
    return normalize(mul(tanNormal, tanToTarget));
}

float4 SampleSkyBox(float3 viewDir, float3 lowColor, float3 highColor)
{
    float viewDot = dot(normalize(viewDir + float3(0, 0.2f, 0)), float3(0, 1, 0));

    float3 colorDown = lerp(float3(0, 0, 0), lowColor, smoothstep(-1.0f, 0.0f, viewDot));
    float3 colorUp = lerp(lowColor, highColor, smoothstep(0, 0.5f, viewDot));

    return float4(lerp(colorDown, colorUp, smoothstep(-1.0f, 0.25f, viewDot)), 1);
}

float Attenuate(uniform float attConst, uniform float attLin, uniform float attQuad, const in float distFragToL)
{
    return 1.0f / (attConst + attLin * distFragToL + attQuad * (distFragToL * distFragToL));
}

float3 Diffuse(
    uniform float3 diffuseColor,
    uniform float diffuseIntensity,
    const in float att,
    const in float3 viewDirFragToL,
    const in float3 viewNormal)
{
    return diffuseColor * diffuseIntensity * att * max(0.0f, dot(viewDirFragToL, viewNormal));
}

float3 Speculate(
    const in float3 specularColor,
    uniform float specularIntensity,
    const in float3 viewNormal,
    const in float3 viewFragToL,
    const in float3 viewPos,
    const in float att,
    const in float specularPower)
{
    // calculate reflected light vector
    const float3 w = viewNormal * dot(viewFragToL, viewNormal);
    const float3 r = normalize(w * 2.0f - viewFragToL);
    // vector from camera to fragment (in view space)
    const float3 viewCamToFrag = normalize(viewPos);
    // calculate specular component color based on angle between
    // viewing vector and reflection vector, narrow with power function
    return att * specularColor * specularIntensity * pow(max(0.0f, dot(-r, viewCamToFrag)), specularPower);
}



struct WaveParameters
{
	float amplitude;
	float frequency;
	float speed;
	float2 direction;
};

struct WaterOutput
{
	float3 Position;
	float3 Normal;
	float3 Tangent;
	float3 BiTangent;
};

float3 calculateWavePosition(float3 position, float _time)
{
	float Q = 0.4f;
	const int numWaves = 4;

	const WaveParameters _WaveParameters[numWaves] = {
		{0.4, 0.2f, 0.001f, float2(0.2, 1)},
		{0.4, 0.32f, 0.00039f, float2(-0.6, -0.2)},
		{0.5, 0.5f, 0.002f, float2(0.4, 0.5)},
		{0.2f, 1.2f, 0.004f, float2(-0.2, -0.3)}
	};

	float yOffset = 0;
	for (int i = 0; i < numWaves; ++i)
	{
		const WaveParameters waveParameters = _WaveParameters[i];
		yOffset += waveParameters.amplitude * sin(waveParameters.frequency * dot(waveParameters.direction, position.xz) + _time * waveParameters.speed);
	}

	float2 xzOffset = 0;
	for (int i = 0; i < numWaves; ++i)
	{
		const WaveParameters waveParameters = _WaveParameters[i];
		float q = Q / (waveParameters.frequency * waveParameters.amplitude * numWaves);

		xzOffset += q * waveParameters.amplitude * waveParameters.direction * cos(waveParameters.frequency * dot(waveParameters.direction, position.xz) + _time * waveParameters.speed);
	}

	return float3(position.x + xzOffset.x, position.y + yOffset, position.z + xzOffset.y);
}

float3 calculateWaveNormal(float3 position, float _time)
{
	float Q = 0.4f;
	const int numWaves = 4;

	const WaveParameters _WaveParameters[numWaves] = {
		{0.4, 0.2f, 0.001f, float2(0.2, 1)},
		{0.4, 0.32f, 0.00039f, float2(-0.6, -0.2)},
		{0.5, 0.5f, 0.002f, float2(0.4, 0.5)},
		{0.2f, 1.2f, 0.004f, float2(-0.2, -0.3)}
	};

	float y = 0;
	for (int i = 0; i < numWaves; ++i)
	{
		const WaveParameters waveParameters = _WaveParameters[i];
		float q = Q / (waveParameters.frequency * waveParameters.amplitude * numWaves);
		y += q * waveParameters.frequency * waveParameters.amplitude * sin(waveParameters.frequency * dot(waveParameters.direction, position.xz) + _time * waveParameters.speed);
	}

	float2 xz = 0;
	for (int i = 0; i < numWaves; ++i)
	{
		const WaveParameters waveParameters = _WaveParameters[i];
		xz += waveParameters.direction * waveParameters.frequency * waveParameters.amplitude * cos(waveParameters.frequency * dot(waveParameters.direction, position.xz) + _time * waveParameters.speed);
	}

	return float3(-xz.x, 1 - y, -xz.y);
}

float3 calculateWaveTangent(float3 position, float _time)
{
	float Q = 0.4f;
	const int numWaves = 4;

	const WaveParameters _WaveParameters[numWaves] = {
		{0.4, 0.2f, 0.001f, float2(0.2, 1)},
		{0.4, 0.32f, 0.00039f, float2(-0.6, -0.2)},
		{0.5, 0.5f, 0.002f, float2(0.4, 0.5)},
		{0.2f, 1.2f, 0.004f, float2(-0.2, -0.3)}
	};

	float x = 0;
	for (int i = 0; i < numWaves; ++i)
	{
		const WaveParameters waveParameters = _WaveParameters[i];
		float q = Q / (waveParameters.frequency * waveParameters.amplitude * numWaves);
		x += q * waveParameters.direction.x * waveParameters.direction.y * waveParameters.frequency * waveParameters.amplitude *
			sin(waveParameters.frequency * dot(waveParameters.direction, position.xz) + _time * waveParameters.speed);
	}

	float y = 0;
	for (int i = 0; i < numWaves; ++i)
	{
		const WaveParameters waveParameters = _WaveParameters[i];
		y += waveParameters.direction.y * waveParameters.frequency * waveParameters.amplitude *
			cos(waveParameters.frequency * dot(waveParameters.direction, position.xz) + _time * waveParameters.speed);
	}

	float z = 0;
	for (int i = 0; i < numWaves; ++i)
	{
		const WaveParameters waveParameters = _WaveParameters[i];
		float q = Q / (waveParameters.frequency * waveParameters.amplitude * numWaves);
		z += q * waveParameters.direction.y * waveParameters.direction.y * waveParameters.frequency * waveParameters.amplitude *
			sin(waveParameters.frequency * dot(waveParameters.direction, position.xz) + _time * waveParameters.speed);
	}
	return float3(-x, y, 1 - z);
}

WaterOutput calculateWaterOutput(float3 modelPosition, float _time)
{
	WaterOutput waterOutput;

	float3 newPosition = calculateWavePosition(modelPosition.xyz, _time);
	newPosition.y += -1.5f;
	//newPosition

	waterOutput.Position = newPosition;
	waterOutput.Normal = calculateWaveNormal(modelPosition.xyz, _time);
	waterOutput.Tangent = calculateWaveTangent(modelPosition.xyz, _time);
	waterOutput.BiTangent = cross(waterOutput.Tangent, waterOutput.Normal);

	return waterOutput;
}