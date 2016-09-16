#include "Common.hlsl"

// Input structs for vertex and pixel shader
struct VS_INPUT
{
	float3 mModelPos	: POSITION;
	float3 mModelNormal	: NORMAL;
	float2 mTex			: TEXCOORD0;
};

struct PS_INPUT
{
	float4 mHomogenousPos	: SV_POSITION;
	float3 mVSNormal		: NORMAL;
	float2 mTex				: TEXCOORD0;
};

struct PS_OUTPUT
{
	float4 m_lightAccumulation : SV_Target0;
	float4 m_diffuse		   : SV_Target1;
	half2  m_normal			   : SV_Target2;
	float4 m_specular		   : SV_Target3;
};

// Encodes a 3-component normal using a spheremap transform into
// a two-component half-precision pair in the range [0-1].
// Assumes the incoming normal is normalized and in view space.
// See: http://aras-p.info/texts/CompactNormalStorage.html
half2 EncodeNormal(float3 inVSNormal)
{
	half p = sqrt(inVSNormal.z * 8.0 + 8.0);
	return half2(inVSNormal.xy / p + 0.5);
}

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT psInput;

	psInput.mHomogenousPos = mul(float4(input.mModelPos, 1.0f), g_objectToProjectionMatrix);
	psInput.mTex = input.mTex;
	psInput.mVSNormal = mul(float4(input.mModelNormal, 0.0f), g_objectToViewMatrix).xyz;

	return psInput;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
PS_OUTPUT PS(PS_INPUT input)
{
	PS_OUTPUT output;

	float3 finalLightAccumulation;

	float3 finalAmbientColor = g_ambientColor.rgb;
	float3 finalEmissiveColor = g_material.m_emissiveColor;
	float3 finalDiffuseColor = g_material.m_diffuseColor;
	float3 finalSpecularColor = g_material.m_specularColor;
	float  finalSpecularPower = g_material.m_specularPower;

	input.mVSNormal = normalize(input.mVSNormal);

	if (g_material.m_bHasEmissiveTex)
	{
		finalEmissiveColor *= g_emissiveMap.Sample(g_linearSampler, input.mTex).rgb;
	}

	if (g_material.m_bHasDiffuseTex)
	{
		finalDiffuseColor *= g_diffuseMap.Sample(g_linearSampler, input.mTex).rgb;
	}

	if (g_material.m_bHasSpecularTex)
	{
		float4 specularSample = g_specularMap.Sample(g_linearSampler, input.mTex).rgba;
		finalSpecularColor *= specularSample.rgb;
		finalSpecularPower *= specularSample.a;
	}

	finalAmbientColor *= finalDiffuseColor;
	finalLightAccumulation = finalAmbientColor + finalEmissiveColor;

	output.m_lightAccumulation = float4(saturate(finalLightAccumulation), 1.0f);
	output.m_diffuse = float4(saturate(finalDiffuseColor), 1.0f);
	output.m_normal = EncodeNormal(input.mVSNormal);
	output.m_specular = saturate(float4(finalSpecularColor, log2(finalSpecularPower) / 10.5f));

	return output;
}
