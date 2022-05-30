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

Texture2D normalMap;
Texture2D worldTexture : register(t1);
Texture2D reflectedWorldTexture : register(t2);

SamplerState splr;

float4 main(float4 Position : SV_Position, float4 WorldPosition : Position, float3 Normal : Normal, float3 Tangent : Tangent, float3 BiTangent : Bitangent, float2 tc : Texcoord) : SV_Target
{
	const float refractionStrength = 0.01f;
	const float reflectionDistortionStrength = 0.01f;
	const float refractionLerpAmount = 0.5f;

	const float3 lightColor = float3(0.7, 0.83, 0.93);
	const float3 ambient = float3(0.35, 0.35, 0.35);
	const float3 waterColor1 = float3(0.05, 0.36, 0.61);
	const float3 waterColor2 = float3(0.45, 0.8, 0.95);

	float2 flippedYPosition = float2(Position.x, 1 - Position.y);

	const float2 screenPosition = Position / viewportSize;
	const float2 flippedYscreenPosition = flippedYPosition / viewportSize;


	const float3x3 tanToTarget = float3x3(normalize(Tangent), normalize(Normal), normalize(BiTangent));
	float3 normalMap1 = normalMap.Sample(splr, WorldPosition.xz * 0.01f + float2(time * 0.00001f, -time * 0.00002f)).rbg * 2 - 1;
	normalMap1.x *= -1;

	float3 normalMap2 = normalMap.Sample(splr, 0.351f + WorldPosition.xz * 0.005f + float2(time * 0.000007f, -time * 0.00001f)).rbg * 2 - 1;
	normalMap2.x *= -1;

	const float3 totalNormal = lerp(normalMap2, normalMap1, smoothstep(-2.7f, 2.7f, WorldPosition.y));
	float3 normal = normalize(mul(totalNormal, tanToTarget));

	const float4 refraction = worldTexture.Sample(splr, screenPosition + normal.rb * refractionStrength);
	const float4 worldColor = worldTexture.Sample(splr, screenPosition);
	const float3 maskedRefraction = worldColor * refraction.a + refraction * (1 - refraction.a);

	float3 reflectedColor = reflectedWorldTexture.Sample(splr, flippedYscreenPosition + normal.rb * reflectionDistortionStrength);

	const float3 lightDirection = normalize(lightPos - WorldPosition);
	
	const float diffuse = saturate(dot(normal, lightDirection));
	const float3 diffuseTotal = lightColor * diffuse * diffuseIntensity;

	const float3 cameraToPosition = normalize(cameraPosition - WorldPosition.xyz);
	const float3 R = reflect(lightDirection, normal);
	const float specular = 0.2f * pow(saturate(dot(R, -cameraToPosition)), 80.5f);
	
	const float3 waterColorTotal = lerp(waterColor1, waterColor2, smoothstep(-2.7f, 2.7f, WorldPosition.y));
	const float3 waterColorShaded = (diffuseTotal + ambient) * waterColorTotal;

	const float3 refractionAndShadedColor = lerp(waterColorShaded, reflectedColor, refractionLerpAmount);

	return float4(refractionAndShadedColor + specular, 1);
}