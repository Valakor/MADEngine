#include "Common.hlsl"

//=>:(Usage, VS, vs_5_0)
//=>:(Usage, PS, ps_5_0)
//=>:(Permute, DIFFUSE)
//=>:(Permute, OPACITY_MASK)

// Input structs for vertex and pixel shader
struct VS_INPUT
{
	float3 mModelPos     : POSITION;
	float3 mModelNormal  : NORMAL;
	float2 mTex          : TEXCOORD;
};

struct PS_INPUT
{
	float4 mHomogenousPos	: SV_POSITION;
	float3 mWSPosition		: POSITION;
	float3 mVSNormal		: NORMAL;
	float2 mTex				: TEXCOORD;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT psInput;

	psInput.mHomogenousPos = mul(float4(input.mModelPos, 1.0f), g_objectToProjectionMatrix);
	psInput.mWSPosition = mul(float4(input.mModelPos, 1.0f), g_objectToWorldMatrix).xyz;
	psInput.mTex = input.mTex;
	psInput.mVSNormal = mul(float4(input.mModelNormal, 0.0f), g_objectToViewMatrix).xyz;
	return psInput;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_Target
{
#ifdef OPACITY_MASK
	float opacityMask = g_opacityMask.Sample(g_anisotropicSampler, input.mTex).r;
	clip(opacityMask < 0.75 ? -1 : 1);
#endif

	float3 finalVSNormal = normalize(input.mVSNormal);
	float3	finalDiffuseColor = g_material.m_diffuseColor;
	float	finalReflectivity = g_material.m_reflectivity;
#ifdef DIFFUSE
	finalDiffuseColor *= g_diffuseMap.Sample(g_anisotropicSampler, input.mTex).rgb;
#endif
	float3 cameraWS = g_cameraInverseViewMatrix[3].xyz;
	float3 skySphereColor = g_cubeMap.Sample(g_linearSampler, input.mWSPosition - cameraWS);
	finalDiffuseColor = lerp(finalDiffuseColor, skySphereColor, finalReflectivity);

	return float4(finalDiffuseColor, 1.0f);
}
