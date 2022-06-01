#include "ShaderOps.hlsl"
#include "PointLightVectorData.hlsl"

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

cbuffer ObjectCBuf
{
    float specularIntensity;
    float specularPower;
    bool normalMapEnabled;
    float padding[1];
};

Texture2D albedoTexture;
Texture2D nmap : register(t1);

SamplerState splr;


float4 main(float4 pos : SV_Position, float3 viewFragPos : Position, float3 viewNormal : Normal, float3 viewTan : Tangent, float3 viewBitan : Bitangent, float2 tc : Texcoord) : SV_Target
{
	viewNormal = MapNormal(normalize(viewTan), normalize(viewBitan), viewNormal, tc, nmap, splr);

	// fragment to light vector data
    const PointLightVectorData lv = CalculateLightVectorData(point_light.pos, viewFragPos);
	// attenuation
    const float att = Attenuate(point_light.attConst, point_light.attLin, point_light.attQuad, lv.distToL);

	// diffuse
    float3 diffuse = Diffuse(point_light.diffuseColor, point_light.diffuseIntensity, att, lv.dirToL, viewNormal);
	diffuse += Diffuse(directional_light.diffuseColor, directional_light.diffuseIntensity, 1, normalize(directional_light.direction), viewNormal);

    // specular
    float3 specular = Speculate(
		point_light.diffuseColor, point_light.diffuseIntensity, viewNormal,
        lv.vToL, viewFragPos, att, specularPower
    );

	specular += Speculate(
		directional_light.diffuseColor, directional_light.diffuseIntensity, viewNormal,
		normalize(directional_light.direction), viewFragPos, 1, specularPower
	);

	// final color
    return float4(saturate((diffuse + directional_light.ambient) * albedoTexture.Sample(splr, tc).rgb + specular), 1.0f);
}