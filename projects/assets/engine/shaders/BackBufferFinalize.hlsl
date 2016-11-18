#include "Common.hlsl"

//=>:(Usage, VS, vs_5_0)
//=>:(Usage, PS, ps_5_0)

// Input structs for vertex and pixel shader
struct VS_INPUT
{
	float3 mPos : POSITION;
};

struct PS_INPUT
{
	float4 mPos : SV_POSITION;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output;
	output.mPos = float4(input.mPos, 1.0);
	return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_Target
{
	int3 texCoord = int3(input.mPos.xy, 0);

	float3 sampleColor = g_lightingBuffer.Load(texCoord).rgb;

	// Exposure tone mapping
	float3 finalColor = 1.0f - exp(-sampleColor * g_cameraExposure);

	// Reinhard tone mapping
	//float3 finalColor = sampleColor / (sampleColor + 1.0f);

	// TODO This is where we would do HDR, tonemapping, etc.
	return float4(finalColor, 1.0f);
}
