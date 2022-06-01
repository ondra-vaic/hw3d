float4 main(float4 pos : SV_Position) : SV_Target
{
	float depthValue = pos.z / pos.w;
	float depth = 1 - smoothstep(0, 0.2f, depthValue);
	return depth;
}