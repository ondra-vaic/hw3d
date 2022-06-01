float4 main(float4 pos : SV_Position) : SV_Target
{
	float4 color;
	float depthValue = pos.z / pos.w;

	if (depthValue < 0.9f)
	{
		color = float4(1.0, 0.0f, 0.0f, 1.0f);
	}

	// The next 0.025% portion of the depth buffer color green.
	if (depthValue > 0.9f)
	{
		color = float4(0.0, 1.0f, 0.0f, 1.0f);
	}

	// The remainder of the depth buffer color blue.
	if (depthValue > 0.925f)
	{
		color = float4(0.0, 0.0f, 1.0f, 1.0f);
	}

	float lineardepth = depthValue;
	// - (0.5f * 500) / (500.0f - depthValue * (500.0f - 0.5f));
	float depth = smoothstep(0, 0.1, pos.z / pos.w);

	return float4(lineardepth, lineardepth, lineardepth, 1);
}