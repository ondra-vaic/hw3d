cbuffer CBuf : register(b1)
{
	float4 color;
};

float4 main(float4 Position : SV_Position, float4 Position2 : Position, float3 Normal : Normal, float3 Tangent : Tangent, float3 BiTangent : Bitangent) : SV_Target
{
	float3 normal = normalize(Normal);
	float3 lightDirection = normalize(float3(0, -1, -1));
	float3 lightColor = float3(0.6, 0.6, 0.6);
	float colorIntensity = dot(normal, -lightDirection);

	return float4(lightColor * colorIntensity, 1);
}