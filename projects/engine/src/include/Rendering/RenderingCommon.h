#pragma once

#define WIN32_LEAN_AND_MEAN
#include <d3d11_1.h>
__pragma(warning(push))
__pragma(warning(disable:4838))
#include <DirectXTK/SimpleMath.h>
__pragma(warning(pop))

namespace MAD
{
	enum class EConstantBufferSlot
	{
		PerScene = 0,
		PerFrame,
		PerPointLight,
		PerDirectionalLight,
		PerMaterial,
		PerDraw,

		MAX
	};

	enum class ETextureSlot
	{
		// ------------- Defined by materials ------------------
		DiffuseMap = 0,
		SpecularMap,
		EmissiveMap,
		// ------------- Defined by renderer -------------------
		DepthBuffer,
		NormalBuffer,
		SpecularBuffer
	};

	enum class ESamplerSlot
	{
		Linear = 0,
		Trilinear,
		Point,
		Anisotropic
	};

	// Lights -------------------------------
	struct SGPUPointLight
	{
		DirectX::SimpleMath::Vector3 m_lightPosition;
		float m_lightRadius;
		DirectX::SimpleMath::Color m_lightColor;
		float m_lightIntensity;
	};

	struct SGPUDirectionalLight
	{
		DirectX::SimpleMath::Vector3 m_lightDirection;
		DirectX::SimpleMath::Color m_lightColor;
		float m_lightIntensity;
	};

	// Materials --------------------------------
	struct SGPUMaterial
	{
		DirectX::SimpleMath::Vector3 m_diffuseColor;

	private:
		float __pad1 = 0.0f;

		// 16 bytes ---------------------------------

	public:
		DirectX::SimpleMath::Vector3 m_specularColor;
		float m_specularPower = 1.0f;

		// 16 bytes ---------------------------------

		DirectX::SimpleMath::Vector3 m_emissiveColor;

	private:
		float __pad2 = 0.0f;

		// 16 bytes ---------------------------------

	public:
		BOOL m_bHasDiffuseTex = FALSE;
		BOOL m_bHasSpecularTex = FALSE;
		BOOL m_bHasEmissiveTex = FALSE;

	private:
		float __pad3 = 0.0f;

		// 16 bytes ---------------------------------
	};
	static_assert(sizeof(SGPUMaterial) == 64, "");

	// Constant Buffers ----------------------
	struct SPerFrameConstants
	{
		DirectX::SimpleMath::Matrix m_cameraViewMatrix;
		DirectX::SimpleMath::Matrix m_cameraProjectionMatrix;
	};

	struct SPerSceneConstants
	{
		DirectX::SimpleMath::Color m_ambientColor;
	};

	struct SPerMaterialConstants
	{
		SGPUMaterial m_material;
	};

	struct SPerDrawConstants
	{
		DirectX::SimpleMath::Matrix m_objectToWorldMatrix;
	};

	struct SPerPointLightConstants
	{
		SGPUPointLight m_pointLight;
	};

	struct SPerDirectionalLightConstants
	{
		SGPUDirectionalLight m_directionalLight;
	};
}