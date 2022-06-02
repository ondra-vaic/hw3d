float4 main(float4 pos : SV_Position, float4 worldPos : Color, float3 normal : Color1) : SV_Target
{
	return float4(worldPos.xyz, saturate(dot(normalize(normal.xyz), float3(0, 1, 0))));
}