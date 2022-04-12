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
	float padding;
};

float4 main(float4 Position : SV_Position, float4 WorldPosition : Position, float3 Normal : Normal, float3 Tangent : Tangent, float3 BiTangent : Bitangent) : SV_Target
{
	float3 normal = normalize(Normal);
	float3 lightDirection = normalize(lightPos - WorldPosition);
	float3 lightColor = float3(0.7, 0.83, 0.93);
	float3 ambient = float3(0.15, 0.15, 0.15);
	float3 waterColor = float3(0.05, 0.36, 0.61);

	float diffuse = saturate(dot(normal, lightDirection));

	float3 cameraToPosition = normalize(cameraPosition - WorldPosition.xyz);
	float3 R = reflect(lightDirection, normal);
	float specular = 0.2f * pow(saturate(dot(R, -cameraToPosition)), 80.5f);

	float3 diffuseTotal = lightColor * diffuse * diffuseIntensity;

	return float4((diffuseTotal + ambient) * waterColor + specular, 1);
}