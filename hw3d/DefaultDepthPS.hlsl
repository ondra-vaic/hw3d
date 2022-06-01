float4 main(float4 pos : SV_Position) : SV_Target
{
	float depthValue = pos.z / pos.w;
	float depth = 1 - smoothstep(1/90.f, 1/25.f, depthValue);
	return depth;
}