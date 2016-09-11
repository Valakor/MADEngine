#include "Common.hlsl"

// Input structs for vertex and pixel shader
struct VS_INPUT
{
	float3 mModelPos : POSITION;
	float3 mModelNormal : NORMAL;
	float2 mTex : TEXCOORD0;
};

struct PS_INPUT
{
	float4 mHomogenousPos : SV_POSITION;
	float3 mVSNormal;
	float2 mTex;
};

struct PS_OUTPUT
{
	float4 m_lightAccumulation : SV_Target0;
	float4 m_diffuse		   : SV_Target1;
	float2 m_normal			   : SV_Target2;
	float4 m_specular		   : SV_Target3;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT psInput;

	psInput.mHomogenousPos = mul(mul(float4(input.mModelPos, 1.0f), g_objectToWorldMatrix), g_cameraViewProjectionMatrix);
	psInput.mTex = input.mTex;
	psInput.mVSNormal = mul(mul(float4(input.mModelNormal, 0.0f), g_objectToWorldMatrix), g_cameraViewMatrix);

	return psInput;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
PS_OUTPUT PS(PS_INPUT input)
{
	PS_OUTPUT output;

	float4 finalLightAccumulation = g_material.m_emissiveColor;
	float3 finalDiffuseColor = g_material.m_diffuseColor;
	float3 finalSpecularColor = g_material.m_specularColor;

	input.mVSNormal = normalize(input.mVSNormal);

	if (g_material.m_bHasEmissiveTex)
	{
		finalLightAccumulation *= g_emissiveMap.Sample(g_linearSampler, input.mTex);
	}

	finalLightAccumulation += g_ambientColor;

	if (g_material.m_bHasDiffuseTex)
	{
		finalDiffuseColor *= g_diffuseMap.Sample(g_linearSampler, input.mTex);
	}

	if (g_material.m_bHasSpecularTex)
	{
		finalSpecularColor *= g_specularMap.Sample(g_linearSampler, input.mTex);
	}

	output.m_lightAccumulation = saturate(float4(finalLightAccumulation, 1.0f));
	output.m_diffuse = saturate(float4(finalDiffuseColor, 1.0f));
	output.m_normal = input.mVSNormal.xy;
	output.m_specular = saturate(float4(finalSpecularColor, log2(g_material.m_specularPower) / 10.5f));

	return output;
}
