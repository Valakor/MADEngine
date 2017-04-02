#include "Common.hlsl"

//=>:(Usage, VS, vs_5_0)
//=>:(Usage, PS, ps_5_0)

// Input structs for vertex and pixel shader
struct VS_INPUT
{
		float3 mTextQuadPos : POSITION;
		float2 mTextUV		: TEXCOORD;
};

struct PS_INPUT
{
		float4 mTextClipPos	: SV_POSITION;
		float2 mTextUV		: TEXCOORD;
};

PS_INPUT VS(VS_INPUT vsInput)
{
	PS_INPUT psInput;
	
	// Depending if 2D or 3D text, the MVP matrix will differ (i.e. for 2D, the MVP matrix will be identity because mTextQuadPos will already be in NDC)
	psInput.mTextClipPos = mul(float4(vsInput.mTextQuadPos, 1.0), g_objectToProjectionMatrix);
	psInput.mTextUV = vsInput.mTextUV;
	
	return psInput;
}

float4 PS(PS_INPUT psInput) : SV_Target
{
	return g_diffuseMap.Sample(g_pointSampler, psInput.mTextUV);
}
