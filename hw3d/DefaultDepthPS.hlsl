float4 main(float4 pos : SV_Position, float4 worldPos : Color) : SV_Target
{
	return float4(worldPos.xyz, 1);
}