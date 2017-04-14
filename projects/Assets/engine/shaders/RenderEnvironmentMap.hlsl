#include "Common.hlsl"

//=>:(Usage, VS, vs_5_0)
//=>:(Usage, PS, ps_5_0)
//=>:(Permute, DIFFUSE)
//=>:(Permute, OPACITY_MASK)
//=>:(Permute, NORMAL_MAP)

// Input structs for vertex and pixel shader
struct VS_INPUT
{
	float3 mModelPos     : POSITION;
	float3 mModelNormal  : NORMAL;
#ifdef NORMAL_MAP
	float4 mModelTangent : TANGENT;
#endif
	float2 mTex          : TEXCOORD;
};

struct PS_INPUT
{
	float4 mHomogenousPos	: SV_POSITION;
	float3 mWSPosition		: POSITION;
	float3 mVSNormal		: NORMAL0;
#ifdef NORMAL_MAP
	float3 mVSTangent		: NORMAL1;
	float3 mVSBitangent		: NORMAL2;
#endif
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
#ifdef NORMAL_MAP
	psInput.mVSTangent = mul(float4(input.mModelTangent.xyz, 0.0f), g_objectToViewMatrix).xyz;
	psInput.mVSBitangent = cross(psInput.mVSNormal, psInput.mVSTangent.xyz) * input.mModelTangent.w;
#endif

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

#ifdef NORMAL_MAP
	float3x3 TBN = float3x3(normalize(input.mVSTangent), normalize(input.mVSBitangent), normalize(input.mVSNormal));
	float3 sampleNormal = g_normalMap.Sample(g_anisotropicSampler, input.mTex).xyz;
	sampleNormal = normalize(sampleNormal * 2.0f - 1.0f);
	float3 finalVSNormal = normalize(mul(sampleNormal, TBN));
#else
	float3 finalVSNormal = normalize(input.mVSNormal);
#endif
	float3	finalDiffuseColor = g_material.m_diffuseColor;
#ifdef DIFFUSE
	finalDiffuseColor *= g_diffuseMap.Sample(g_anisotropicSampler, input.mTex).rgb;
#endif
	return float4(finalDiffuseColor, 1.0f);
}
