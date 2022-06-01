#include "ShaderOps.hlsl"

cbuffer LightData
{
	struct PointLight
	{
		float3 pos;
		float3 diffuseColor;
		float diffuseIntensity;
		float attConst;
		float attLin;
		float attQuad;
	}point_light;

	struct DirectionalLight
	{
		float3 direction;
		float3 ambient;

		float3 diffuseColor;
		float diffuseIntensity;
	}directional_light;
};

cbuffer CBuf : register(b1)
{
	float3 cameraPosition;
	float3 mainCloseColor;
	float3 mainFarColor;

	float time;
	float2 viewportSize;
};

cbuffer SkyBox : register(b3)
{
	float3 LowColor;
	float3 HighColor;

	float3 CameraPosition;
};

Texture2D normalTexture;
Texture2D worldTexture : register(t1);
Texture2D reflectedWorldTexture : register(t2);
Texture2D depthTexture : register(t3);
Texture2D causticsLUTexture : register(t4);

SamplerState splr;

half Fresnel(half nDotL, half fresnelBias, half fresnelPow)
{
	half facing = (1.0 - nDotL);
	return max(fresnelBias + (1.0 - fresnelBias) * pow(facing, fresnelPow), 0.0);
}

float calculateSpecular(float3 viewVector, float3 lightDirection, float3 normal)
{
	const float3 R = reflect(lightDirection, normal);
	return 0.5f * pow(saturate(dot(R, -viewVector)), 80.5f);
}

float calculateDiffuse(float3 lightDirection, float3 normal, float diffuseIntensity)
{
	return saturate(dot(normal, lightDirection)) * diffuseIntensity;
}

float3 calculateColor(float4 worldPosition, float3 normal, float3 cameraToPosition, float depth)
{
	const float3 pointLightDirection = normalize(point_light.pos - worldPosition);
	const float3 directionalLightDirection = normalize(directional_light.direction);

	float3 depthLerpedColor = lerp(mainCloseColor, mainFarColor, depth);

	float3 diffuseTotal = calculateDiffuse(directionalLightDirection, normal, directional_light.diffuseIntensity) * depthLerpedColor;
	diffuseTotal += calculateDiffuse(pointLightDirection, normal, point_light.diffuseIntensity) * depthLerpedColor;

	float3 specular = calculateSpecular(cameraToPosition, directionalLightDirection, normal) *
		SampleSkyBox(reflect(-directionalLightDirection, normal), LowColor, HighColor);
	specular += calculateSpecular(cameraToPosition, pointLightDirection, normal) *
		SampleSkyBox(reflect(-pointLightDirection, normal), LowColor, HighColor);

	const float3 waterColorShaded = (diffuseTotal + directional_light.ambient);

	return waterColorShaded + specular;
}

float3 calculateNormal(float3 worldPosition, float3 normal, float3 tangent, float3 biTangent : Bitangent)
{
	const float3x3 tanToTarget = float3x3(normalize(tangent), normalize(normal), normalize(biTangent));
	float3 normalMap1 = normalTexture.Sample(splr, worldPosition.xz * 0.05f + float2(time * 0.00001f, -time * 0.00002f)).rbg * 2 - 1;
	normalMap1.x *= -1;

	float3 normalMap2 = normalTexture.Sample(splr, 0.351f + worldPosition.xz * 0.016f + float2(time * 0.000007f, -time * 0.00001f)).rbg * 2 - 1;
	normalMap2.x *= -1;

	const float3 totalNormal = lerp(normalMap2, normalMap1, smoothstep(-2.7f, 2.7f, worldPosition.y));
	return normalize(mul(totalNormal, tanToTarget));
}

float4 main(float4 Position : SV_Position, float4 WorldPosition : Position, float4 ModelPosition : Color, float3 Normal : Normal, float3 Tangent : Tangent, float3 BiTangent : Bitangent, float2 UV : Texcoord) : SV_Target
{
	const float3 cameraToPosition = normalize(cameraPosition - WorldPosition.xyz);

	const float refractionDistortionStrength = 0.01f;
	const float reflectionDistortionStrength = 0.02f;

	float2 flippedYPosition = float2(Position.x, 1 - Position.y);

	const float2 screenPosition = Position / viewportSize;
	const float2 flippedYscreenPosition = flippedYPosition / viewportSize;

	float3 normal = calculateNormal(WorldPosition, Normal, Tangent, BiTangent);

	float3 vRefrBump = normalize(normal);
	float3 vReflBump = normalize(normal);

	float3 environmentPositionRefracted = depthTexture.Sample(splr, screenPosition + vRefrBump * refractionDistortionStrength);
	float3 environmentPositionRefractedLess = depthTexture.Sample(splr, screenPosition + vRefrBump * refractionDistortionStrength);
	float distanceToEnvironmentRefracted = smoothstep(0, 0.5f, length(cameraPosition - environmentPositionRefracted) / 500);

	float3 environmentPosition = depthTexture.Sample(splr, screenPosition);
	float distanceToEnvironment = smoothstep(0, 0.5f, length(cameraPosition - environmentPosition) / 500);

	float distanceToWater = smoothstep(0, 0.5f, length(cameraPosition - WorldPosition.xyz) / 500);

	float3 waterPlanePosition = float3(environmentPositionRefractedLess.x, ModelPosition.y, environmentPositionRefractedLess.z);

	WaterOutput waterOutput = calculateWaterOutput(waterPlanePosition, time);

	float3 normalAboveGround = calculateNormal(waterOutput.Position, waterOutput.Normal, waterOutput.Tangent, waterOutput.BiTangent);
	
	float causticsDot = causticsLUTexture.Sample(splr, normalAboveGround.xy);
	float causticsStrength = saturate(pow(causticsDot * 14, 0.4));

	float3 rand = frac(normalAboveGround * 4.157f);

	float3 col = causticsStrength * normalize(float3(1.0f - rand.x * 0.4f, 0.8f, 0.6f + rand.y * 0.4f));
	
	float3 waterColorRefracted = calculateColor(WorldPosition, normal, cameraToPosition, smoothstep(0, 0.1, distanceToEnvironmentRefracted - distanceToWater));
	float3 waterColor = calculateColor(WorldPosition, normal, cameraToPosition, smoothstep(0, 0.1, distanceToEnvironment - distanceToWater));

	float4 refractionA = worldTexture.Sample(splr, screenPosition + vRefrBump * refractionDistortionStrength);
	float4 refractionB = worldTexture.Sample(splr, screenPosition);
	
	const float4 maskedRefraction = lerp(refractionA, refractionB, smoothstep(0, 0.01, refractionA.a));
	const float3 maskedWaterColor = lerp(waterColorRefracted, waterColor, smoothstep(0, 0.01, refractionA.a));
	
	float4 reflectedColor = reflectedWorldTexture.Sample(splr, flippedYscreenPosition + vReflBump * reflectionDistortionStrength);

	float NdotL = max(dot(cameraToPosition, vReflBump), 0);
	float facing = (1.0 - NdotL);
	float fresnel = Fresnel(NdotL, 0.1, 0.8);
	float3 cReflect = fresnel * reflectedColor;

	float fDistScale = saturate(10.0f/Position.w);
	
	float3 caustics = col * (1 - smoothstep(0, 0.25f, distanceToEnvironmentRefracted - distanceToWater));
	
	float3 WaterDeepColor = lerp(maskedWaterColor, maskedRefraction, fDistScale);
	float3 waterCloseColor = lerp(WaterDeepColor, maskedWaterColor + caustics, facing);

	return float4(cReflect + waterCloseColor, 1);
}