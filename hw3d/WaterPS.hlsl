cbuffer PointLightCBuf
{
	float3 lightPos;
	float3 ambient;
	float3 diffuseColor;
	float diffuseIntensity;
	float attConst;
	float attLin;
	float attQuad;
};

cbuffer CBuf : register(b1)
{
	float3 cameraPosition;
	float time;
	float2 viewportSize;
};

struct WaveParameters
{
	float amplitude;
	float frequency;
	float speed;
	float2 direction;
};

Texture2D tex;
Texture2D worldTexture : register(t1);

SamplerState splr;

float4 main(float4 Position : SV_Position, float4 WorldPosition : Position, float3 Normal : Normal, float3 Tangent : Tangent, float3 BiTangent : Bitangent, float2 tc : Texcoord) : SV_Target
{
	float2 screenPosition = Position.xy / viewportSize;

	return float4(worldTexture.Sample(splr, screenPosition).rgb, 1);

	const float3x3 tanToTarget = float3x3(normalize(Tangent), normalize(Normal), normalize(BiTangent));
	float3 normalMap1 = tex.Sample(splr, WorldPosition.xz * 0.01f + float2(time * 0.00001f, -time * 0.00002f)).rbg * 2 - 1;
	normalMap1.x *= -1;

	float3 normalMap2 = tex.Sample(splr, 0.351f + WorldPosition.xz * 0.005f + float2(time * 0.000007f, -time * 0.00001f)).rbg * 2 - 1;
	normalMap2.x *= -1;

	float3 totalNormal = lerp(normalMap2, normalMap1, smoothstep(-2.7f, 2.7f, WorldPosition.y));

	float3 normal = normalize(mul(totalNormal, tanToTarget));

	float3 lightDirection = normalize(lightPos - WorldPosition);
	float3 lightColor = float3(0.7, 0.83, 0.93);
	float3 ambient = float3(0.35, 0.35, 0.35);
	float3 waterColor1 = float3(0.05, 0.36, 0.61);
	float3 waterColor2 = float3(0.45, 0.8, 0.95);

	float3 waterColorTotal = lerp(waterColor1, waterColor2, smoothstep(-2.7f, 2.7f, WorldPosition.y));

	float diffuse = saturate(dot(normal, lightDirection));

	float3 cameraToPosition = normalize(cameraPosition - WorldPosition.xyz);
	float3 R = reflect(lightDirection, normal);
	float specular = 0.2f * pow(saturate(dot(R, -cameraToPosition)), 80.5f);

	float3 diffuseTotal = lightColor * diffuse * diffuseIntensity;

	return float4((diffuseTotal + ambient) * waterColorTotal + specular, 1);
}