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
	float aa;
	float4 mainCloseColor;
	float4 mainFarColor;
	
	float2 viewportSize;
	float2 normalMap0scrollSpeed = { 0.00001,  0.00002f };
	float2 normalMap1scrollSpeed = { 0.000007f,  0.00001f };

	float time;
	float planeY;
	float specular = 0.5f;
	float specularPower = 80.f;
	float skyBoxWeight = 0.7f;
	float refractionDistortionStrength = 0.1f;
	float reflectionDistortionStrength = 0.2f;
	float fresnelBias = 0.1f;
	float fresnelPower = 0.8f;
	float fresnelDistantScale = 10.0f;
	float causticsPower = 0.3f;
	float causticsLength = 14.1;
	float causticsDepthFallOff = 0.25f;

	float depthFar = 175.0f;
	float depthCutOff = 0.5f;

	float normalMap0Scale = 0.05f;
	float normalMap1Scale = 0.016f;

	float1 pad;
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
	return specular * pow(saturate(dot(R, -viewVector)), specularPower);
}

float calculateDiffuse(float3 lightDirection, float3 normal, float diffuseIntensity)
{
	return saturate(dot(normal, lightDirection)) * diffuseIntensity;
}

float3 calculateColor(float4 worldPosition, float3 normal, float3 cameraToPosition, float depth)
{
	const float3 pointLightDirection = normalize(point_light.pos - worldPosition);
	const float3 directionalLightDirection = normalize(directional_light.direction);

	const float3 depthLerpedColor = lerp(mainCloseColor, mainFarColor, depth);

	const float3 environmentColor = SampleSkyBox(reflect(-cameraToPosition, normal), LowColor, HighColor);
	const float directionEnvironmentBlendDir = dot(reflect(-cameraToPosition, normal), directionalLightDirection);
	const float pointEnvironmentBlendDir = dot(reflect(-cameraToPosition, normal), pointLightDirection);

	const float dirDiffuse = calculateDiffuse(directionalLightDirection, normal, directional_light.diffuseIntensity);
	const float pointDiffuse = calculateDiffuse(pointLightDirection, normal, point_light.diffuseIntensity);

	const float3 dirLightColor = lerp(environmentColor, directional_light.diffuseColor, directionEnvironmentBlendDir);
	const float3 pointLightColor = lerp(environmentColor, point_light.diffuseColor, pointEnvironmentBlendDir);
	
	float3 diffuseTotal = lerp(depthLerpedColor, dirLightColor * dirDiffuse, skyBoxWeight);
	diffuseTotal += lerp(depthLerpedColor, pointLightColor * pointDiffuse, skyBoxWeight);

	float3 specular = calculateSpecular(cameraToPosition, directionalLightDirection, normal) * dirLightColor;
	specular += calculateSpecular(cameraToPosition, pointLightDirection, normal) * pointLightColor;

	const float3 waterColorShaded = (diffuseTotal + directional_light.ambient);

	return waterColorShaded + specular;
}

float3 calculateNormal(float3 worldPosition, float3 normal, float3 tangent, float3 biTangent : Bitangent)
{
	const float3x3 tanToTarget = float3x3(normalize(tangent), normalize(normal), normalize(biTangent));
	float3 normalMap1 = normalTexture.Sample(splr, worldPosition.xz * normalMap0Scale + float2(time * normalMap0scrollSpeed.x, -time * normalMap0scrollSpeed.y)).rbg * 2 - 1;
	normalMap1.x *= -1;

	return normalize(mul(normalMap1, tanToTarget));
}

float3 calculateCaustics(float4 environmentPositionRefracted, float waterDepthRefracted)
{
	const float3 waterPlanePosition = float3(environmentPositionRefracted.x, planeY, environmentPositionRefracted.z);

	const WaterOutput waterOutput = calculateWaterOutput(waterPlanePosition, time);

	float3 normalAboveGround = calculateNormal(waterOutput.Position, waterOutput.Normal, waterOutput.Tangent, waterOutput.BiTangent);

	const float causticsMapSample = causticsLUTexture.Sample(splr, normalAboveGround.xz);
	const float causticsStrength = saturate(pow(causticsMapSample * causticsLength, causticsPower)) * saturate(dot(float3(0, 1, 0), directional_light.direction));

	float3 rand = frac(normalAboveGround * 4.157f);
	float3 chromaColor = normalize(float3(1.0f - rand.x * 0.4f, 0.8f, 0.6f + rand.y * 0.4f));

	return causticsStrength * chromaColor * smoothstep(causticsDepthFallOff, 1, (1 - waterDepthRefracted))* environmentPositionRefracted.a;
}

float4 main(float4 Position : SV_Position, float4 WorldPosition : Position, float3 Normal : Normal, float3 Tangent : Tangent, float3 BiTangent : Bitangent, float2 UV : Texcoord) : SV_Target
{
	const float3 cameraToPosition = normalize(cameraPosition - WorldPosition.xyz);

	const float2 flippedYPosition = float2(Position.x, 1 - Position.y);

	const float2 screenPosition = Position / viewportSize;
	const float2 flippedYscreenPosition = flippedYPosition / viewportSize;

	const float3 normal = calculateNormal(WorldPosition, Normal, Tangent, BiTangent);

	const float3 vRefrBump = normalize(normal);
	const float3 vReflBump = normalize(normal);

	const float2 refractedUV = screenPosition + vRefrBump.xz * refractionDistortionStrength;
	const float4 environmentPositionRefracted = depthTexture.Sample(splr, refractedUV);
	const float4 environmentPosition = depthTexture.Sample(splr, screenPosition);

	const float4 refractionA = worldTexture.Sample(splr, refractedUV);
	const float4 refractionB = worldTexture.Sample(splr, screenPosition);

	const float depthEnvironmentRefracted = length(cameraPosition - environmentPositionRefracted.xyz);
	const float depthEnvironment = length(cameraPosition - environmentPosition.xyz);
	const float distanceWater = length(cameraPosition - WorldPosition.xyz);

	const float waterDepthWorld = smoothstep(0.f, 6.8f, (depthEnvironment - distanceWater));
	const float waveHeightMask = 1 - smoothstep(0, .25f, WorldPosition.y - planeY);

	const float reflectionMask = saturate(environmentPosition.w + smoothstep(0, 0.85, waterDepthWorld) + waveHeightMask * waterDepthWorld * environmentPosition.w);

	const float waterDepth = smoothstep(0, depthCutOff, (depthEnvironment - distanceWater) / depthFar);
	const float waterDepthRefracted = smoothstep(0, depthCutOff, (depthEnvironmentRefracted - distanceWater) / depthFar);
	const float waterDepthMasked = lerp(waterDepthRefracted, waterDepth, smoothstep(0, 0.01, refractionA.a));

	const float4 environmentPositionDepthMasked = lerp(environmentPositionRefracted, environmentPosition, smoothstep(0, 0.01, refractionA.a));

	const float3 caustics = calculateCaustics(environmentPositionDepthMasked, waterDepthMasked);
	const float4 maskedRefraction = lerp(refractionA, refractionB, smoothstep(0, 0.01, refractionA.a));

	float4 reflectedColor = reflectedWorldTexture.Sample(splr, flippedYscreenPosition + vReflBump.xz * reflectionDistortionStrength);
	reflectedColor = lerp(maskedRefraction, reflectedColor, reflectionMask);

	const float NdotL = max(dot(cameraToPosition, vReflBump), 0);
	const float facing = (1.0 - NdotL);
	const float fresnel = Fresnel(NdotL, fresnelBias, fresnelPower);
	const float3 cReflect = fresnel * reflectedColor;

	const float3 waterColorRefracted = calculateColor(WorldPosition, normal, cameraToPosition, smoothstep(0, 0.1, waterDepthMasked));
	const float3 waterColor = calculateColor(WorldPosition, normal, cameraToPosition, smoothstep(0, 0.1, waterDepth));
	const float3 maskedWaterColor = lerp(waterColorRefracted, waterColor, smoothstep(0, 0.01, refractionA.a));

	const float fDistScale = saturate(fresnelDistantScale/Position.w);

	const float3 WaterDeepColor = lerp(maskedWaterColor, maskedRefraction + caustics, fDistScale);
	const float3 waterCloseColor = lerp(WaterDeepColor, maskedWaterColor + caustics, facing);

	return float4(cReflect + waterCloseColor, 1);
}