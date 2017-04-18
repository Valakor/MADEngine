#pragma pack_matrix(row_major)

struct PointLight
{
	float3 m_lightPosition;
	float m_lightIntensity;
	float4 m_lightColor;
	float m_lightInnerRadius;
	float m_lightOuterRadius;

	float4x4 m_viewProjectionMatrix;
};

struct DirectionalLight
{
	float3 m_lightDirection;
	float  m_lightIntensity;
	float4 m_lightColor;

	float4x4 m_viewProjectionMatrix;
};

struct MeshMaterial
{
	float3 m_diffuseColor;
	float3 m_specularColor;
	float  m_specularPower;
	float3 m_emissiveColor;
	float  m_reflectivity;
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
	float4x4 g_cameraInverseViewMatrix;
	float4x4 g_cameraInverseProjectionMatrix;

	float g_cameraNearPlane;
	float g_cameraFarPlane;
	float g_cameraExposure;

	float g_gameTime;
	float g_frameTime;
};

cbuffer CBPerPointLightConstants : register(b2)
{
	PointLight g_pointLight;

	// TEMP: Testing
	float4x4 g_pointLightVPMatrices[6];
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

SamplerState g_pointSampler					: register(s0);
SamplerState g_linearSampler				: register(s1);
SamplerState g_trilinearSampler				: register(s2);
SamplerState g_anisotropicSampler			: register(s3);
SamplerComparisonState g_shadowMapSampler	: register(s4);

Texture2D	g_diffuseMap		: register(t0);
Texture2D	g_specularMap		: register(t1);
Texture2D	g_emissiveMap		: register(t2);
Texture2D	g_opacityMask		: register(t3);
Texture2D	g_normalMap			: register(t4);
TextureCube g_cubeMap			: register(t5);
TextureCube g_globalCubeMap		: register(t6);

Texture2D	g_lightingBuffer	: register(t7);
Texture2D	g_diffuseBuffer		: register(t8);
Texture2D	g_normalBuffer		: register(t9);
Texture2D	g_specularBuffer	: register(t10);
Texture2D	g_depthBuffer		: register(t11);
