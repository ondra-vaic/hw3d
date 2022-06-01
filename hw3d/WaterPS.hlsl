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

Texture2D albedoTexture;
Texture2D worldTexture : register(t1);
Texture2D reflectedWorldTexture : register(t2);
Texture2D depthTexture : register(t3);

SamplerState splr;

half Fresnel(half nDotL, half fresnelBias, half fresnelPow)
{
	half facing = (1.0 - nDotL);
	return max(fresnelBias + (1.0 - fresnelBias) * pow(facing, fresnelPow), 0.0);
}

/*
float refractionAndReflection()
{
	half3 vEye = normalize(Eye);

	// Average bump layers
	half3 vBumpTex=normalize(2.0 * (vBumpTexA.xyz + vBumpTexB.xyz + vBumpTexC.xyz + vBumpTexD.xyz) - 4.0);

	// Apply individual bump scale for refraction and reflection
	half3 vRefrBump = vBumpTex.xyz * half3(0.075, 0.075, 1.0);
	half3 vReflBump = vBumpTex.xyz * half3(0.02, 0.02, 1.0);

	half4 vRefrA = tex2D(tex1, vProj.xy + vRefrBump.xy);
	half4 vRefrB = tex2D(tex1, vProj.xy);
	half4 vReflection = tex2D(tex2, vProj.xy + vReflBump.xy);
	half4 vRefraction = vRefrB * vRefrA.w + vRefrA * (1 - vRefrA.w);

	// Compute Fresnel term
	half NdotL = max(dot(vEye, vReflBump), 0);
	half facing = (1.0 - NdotL);
	half fresnel = Fresnel(NdotL, 0.2, 5.0);

	// Use distance to lerp between refraction and deep water color
	half fDistScale = saturate(10.0/Wave0.w);
	half3 WaterDeepColor = (vRefraction.xyz * fDistScale + (1 - fDistScale) * half3(0, 0.15, 0.115));

	// Lerp between water color and deep water color
	half3 WaterColor = half3(0, 0.15, 0.115);
	half3 waterColor = (WaterColor * facing + WaterDeepColor * (1.0 - facing));
	half3 cReflect = fresnel * vReflection;

	// final water = reflection_color * fresnel + water_color
	return half4(cReflect + waterColor, 1);
}
*/

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
	float3 normalMap1 = albedoTexture.Sample(splr, worldPosition.xz * 0.01f + float2(time * 0.00001f, -time * 0.00002f)).rbg * 2 - 1;
	normalMap1.x *= -1;

	float3 normalMap2 = albedoTexture.Sample(splr, 0.351f + worldPosition.xz * 0.005f + float2(time * 0.000007f, -time * 0.00001f)).rbg * 2 - 1;
	normalMap2.x *= -1;

	const float3 totalNormal = lerp(normalMap2, normalMap1, smoothstep(-2.7f, 2.7f, worldPosition.y));
	return normalize(mul(totalNormal, tanToTarget));
}

float4 main(float4 Position : SV_Position, float4 WorldPosition : Position, float3 Normal : Normal, float3 Tangent : Tangent, float3 BiTangent : Bitangent, float2 UV : Texcoord) : SV_Target
{
	const float3 cameraToPosition = normalize(cameraPosition - WorldPosition.xyz);
	float distanceToWater = 1 - smoothstep(0, 0.2f, length(cameraPosition - WorldPosition.xyz) / 500);

	const float refractionDistortionStrength = 0.02f;
	const float reflectionDistortionStrength = 0.03f;

	float2 flippedYPosition = float2(Position.x, 1 - Position.y);

	const float2 screenPosition = Position / viewportSize;
	const float2 flippedYscreenPosition = flippedYPosition / viewportSize;

	float depth = depthTexture.Sample(splr, screenPosition);

	float3 normal = calculateNormal(WorldPosition, Normal, Tangent, BiTangent);
	float3 waterColor = calculateColor(WorldPosition, normal, cameraToPosition, depth - distanceToWater);

	float3 vRefrBump = normalize(normal * half3(0.075, 0.075, 0.06) * 0.5);
	float3 vReflBump = normalize(normal * half3(0.02, 0.02, 0.06));

	const float4 refractionA = worldTexture.Sample(splr, screenPosition + vRefrBump * refractionDistortionStrength);
	const float4 refractionB = worldTexture.Sample(splr, screenPosition);

	const float4 maskedRefraction = lerp(refractionA, refractionB, smoothstep(0, 0.01, refractionA.a));

	float4 reflectedColorA = reflectedWorldTexture.Sample(splr, flippedYscreenPosition + vReflBump * reflectionDistortionStrength);

	float NdotL = max(dot(cameraToPosition, vReflBump), 0);
	float facing = (1.0 - NdotL);
	float fresnel = Fresnel(NdotL, 0.1, 0.8);
	float3 cReflect = fresnel * reflectedColorA;

	float fDistScale = saturate(16.0f/Position.w);
	
	float3 WaterDeepColor = lerp(waterColor, maskedRefraction, fDistScale);
	float3 waterCloseColor = lerp(WaterDeepColor, waterColor, facing);

	return float4(cReflect + waterCloseColor, 1);
}