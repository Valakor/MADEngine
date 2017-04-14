#include "Common.hlsl"

//=>:(Usage, VS, vs_5_0)
//=>:(Usage, PS, ps_5_0)
//=>:(Permute, DIFFUSE)
//=>:(Permute, SPECULAR)
//=>:(Permute, EMISSIVE)
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

// Encodes a specular power in the range [1-1448.15] to the
// range [0-1]. Provides best precision for values in the
// range [1-256].
// See: http://www.3dgep.com/forward-plus/#Specular_Buffer
float EncodeSpecPower(float inTrueSpecularPower)
{
	return log2(inTrueSpecularPower) / 10.5;
}

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
PS_OUTPUT PS(PS_INPUT input)
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

	float3 finalLightAccumulation;

	float3	finalAmbientColor	= g_ambientColor.rgb;
	float3	finalEmissiveColor	= g_material.m_emissiveColor;
	float3	finalDiffuseColor	= g_material.m_diffuseColor;
	float3	finalSpecularColor	= g_material.m_specularColor;
	float	finalSpecularPower	= g_material.m_specularPower;
	float	finalReflectivity	= g_material.m_reflectivity;

#ifdef EMISSIVE
	finalEmissiveColor *= g_emissiveMap.Sample(g_anisotropicSampler, input.mTex).rgb;
#endif

#ifdef DIFFUSE
	finalDiffuseColor *= g_diffuseMap.Sample(g_anisotropicSampler, input.mTex).rgb;
#endif

#ifdef SPECULAR
	float4 specularSample = g_specularMap.Sample(g_anisotropicSampler, input.mTex).rgba;
	finalSpecularColor *= specularSample.rgb;
	finalSpecularPower *= specularSample.a;
#endif

	float3 normalWS = mul(float4(finalVSNormal, 0.0), g_cameraInverseViewMatrix);
	float3 cameraWS = g_cameraInverseViewMatrix[3].xyz;
	float3 cameraReflectedWS = reflect(input.mWSPosition - cameraWS, normalWS);
	float3 skySphereColor = g_cubeMap.Sample(g_linearSampler, cameraReflectedWS);
	finalDiffuseColor = (finalReflectivity * skySphereColor) + ((1.0 - finalReflectivity) * finalDiffuseColor);

	finalAmbientColor *= finalDiffuseColor;
	finalLightAccumulation = finalAmbientColor + finalEmissiveColor;

	PS_OUTPUT output;
	output.m_lightAccumulation	= float4(saturate(finalLightAccumulation), 1.0f);
	output.m_diffuse			= float4(saturate(finalDiffuseColor), 1.0f);
	output.m_normal				= EncodeNormal(finalVSNormal);
	output.m_specular			= float4(saturate(finalSpecularColor), EncodeSpecPower(finalSpecularPower));

	return output;
}
