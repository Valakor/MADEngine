#pragma pack_matrix(row_major)

struct PointLight
{
	float3 m_lightPosition;
	float  m_lightRadius;
	float4 m_lightColor;
	float  m_lightIntensity;
};

struct DirectionalLight
{
	float3 m_lightDirection;
	float  m_lightIntensity;
	float4 m_lightColor;
};

struct MeshMaterial
{
	float3 m_diffuseColor;
	bool   m_bHasDiffuseTex;
	float3 m_specularColor;
	float  m_specularPower;
	float3 m_emissiveColor;
	bool   m_bHasEmissiveTex;
	bool   m_bHasSpecularTex;
};

cbuffer CBPerSceneConstants : register(b0)
{
	float4 g_ambientColor;
	float2 g_screenDimensions;
}

cbuffer CBPerFrameConstants : register(b1)
{
	float4x4 g_cameraViewMatrix;
	float4x4 g_cameraProjectionMatrix;
	float4x4 g_cameraViewProjectionMatrix;
	float4x4 g_cameraInverseProjectionMatrix;
};

cbuffer CBPerPointLightConstants : register(b2)
{
	PointLight g_pointLight;
};

cbuffer CBPerDirectionalLightConstants : register(b3)
{
	DirectionalLight g_directionalLight;
};

cbuffer CBPerMaterialConstants : register(b4)
{
	MeshMaterial g_material;
}

cbuffer CBPerDrawConstants : register(b5)
{
	float4x4 g_objectToWorldMatrix;
	float4x4 g_objectToViewMatrix;
	float4x4 g_objectToProjectionMatrix;
};

SamplerState g_pointSampler			: register(s0);
SamplerState g_linearSampler		: register(s1);
SamplerState g_trilinearSampler		: register(s2);
SamplerState g_anisotropicSampler	: register(s3);

Texture2D g_diffuseMap	: register(t0);
Texture2D g_specularMap	: register(t1);
Texture2D g_emissiveMap	: register(t2);

Texture2D g_diffuseBuffer	: register(t3);
Texture2D g_normalBuffer	: register(t4);
Texture2D g_specularBuffer	: register(t5);
Texture2D g_depthBuffer		: register(t6);