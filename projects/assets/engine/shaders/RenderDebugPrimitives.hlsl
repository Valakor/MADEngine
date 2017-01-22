#include "Common.hlsl"

//=>:(Usage, VS, vs_5_0)
//=>:(Usage, PS, ps_5_0)

// Input structs for vertex and pixel shader
struct VS_INPUT
{
	float3 mModelPos : POSITION;
	float3 mVertexColor : NORMAL;
};

struct PS_INPUT
{
	float4 mClipPos : SV_POSITION;
	float4 mPixelColor : COLOR;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS(VS_INPUT vsInput)
{
	PS_INPUT psInput;
	
	psInput.mClipPos = mul(float4(vsInput.mModelPos, 1.0), g_objectToProjectionMatrix);
	psInput.mPixelColor = float4(vsInput.mVertexColor, 1.0);
	
	return psInput;
}

float4 PS(PS_INPUT psInput) : SV_Target
{
	return psInput.mPixelColor;
}
