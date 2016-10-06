#include "Common.hlsl"

//=>:(Usage, VS, vs_5_0)

// Input structs for vertex and pixel shader
struct VS_INPUT
{
	float3 mModelPos : POSITION;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
float4 VS(VS_INPUT input) : SV_POSITION
{
	return mul(float4(input.mModelPos, 1.0f), g_objectToProjectionMatrix);
}
