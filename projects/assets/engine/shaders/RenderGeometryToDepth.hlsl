#include "Common.hlsl"

//=>:(Usage, VS, vs_5_0)
//=>:(Permute, DIRECTIONAL_LIGHT)
//=>:(Permute, POINT_LIGHT)

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
#ifdef DIRECTIONAL_LIGHT
	return mul(mul(float4(input.mModelPos, 1.0f), g_objectToWorldMatrix), g_directionalLight.m_viewProjectionMatrix);
#else
	return mul(mul(float4(input.mModelPos, 1.0f), g_objectToWorldMatrix), g_pointLight.m_viewProjectionMatrix);
#endif
}
