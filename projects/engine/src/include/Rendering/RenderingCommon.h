#pragma once

#include "Core/SimpleMath.h"

#include <EASTL/type_traits.h>

namespace MAD
{
#define DECLARE_SLOT_TO_INTEGRAL(SlotEnum) \
	inline constexpr eastl::underlying_type<SlotEnum>::type AsIntegral(SlotEnum e) \
	{ \
		return static_cast<eastl::underlying_type<SlotEnum>::type>(e); \
	}

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
	DECLARE_SLOT_TO_INTEGRAL(EConstantBufferSlot)

	enum class ETextureSlot
	{
		// ------------- Defined by materials ------------------
		DiffuseMap = 0,
		SpecularMap,
		EmissiveMap,

		// ------------- Defined by renderer -------------------
		DiffuseBuffer,
		NormalBuffer,
		SpecularBuffer,
		DepthBuffer,

		MAX
	};
	DECLARE_SLOT_TO_INTEGRAL(ETextureSlot)

	enum class ERenderTargetSlot
	{
		BackBuffer,
		DiffuseBuffer,
		NormalBuffer,
		SpecularBuffer,

		MAX
	};
	DECLARE_SLOT_TO_INTEGRAL(ERenderTargetSlot);

	enum class ESamplerSlot
	{
		Point = 0,
		Linear,
		Trilinear,
		Anisotropic,

		MAX
	};
	DECLARE_SLOT_TO_INTEGRAL(ESamplerSlot)

#undef DECLARE_SLOT_TO_INTEGRAL

	// Lights -------------------------------
	struct SGPUPointLight
	{
		DirectX::SimpleMath::Vector3 m_lightPosition;
		float m_lightRadius;
		DirectX::SimpleMath::Color m_lightColor;
		float m_lightIntensity;

	private:
		float __pad1 = 0.0f;
		float __pad2 = 0.0f;
		float __pad3 = 0.0f;
	};
	static_assert(sizeof(SGPUPointLight) == 48, "");

	struct SGPUDirectionalLight
	{
		DirectX::SimpleMath::Vector3 m_lightDirection;
		float m_lightIntensity;
		DirectX::SimpleMath::Color m_lightColor;
	};
	static_assert(sizeof(SGPUDirectionalLight) == 32, "");

	// Materials --------------------------------
	struct SGPUMaterial
	{
		DirectX::SimpleMath::Vector3 m_diffuseColor;
		BOOL m_bHasDiffuseTex = FALSE;

		// 16 bytes ---------------------------------

		DirectX::SimpleMath::Vector3 m_specularColor;
		float m_specularPower = 1.0f;

		// 16 bytes ---------------------------------

		DirectX::SimpleMath::Vector3 m_emissiveColor;
		BOOL m_bHasEmissiveTex = FALSE;

		// 16 bytes ---------------------------------

		BOOL m_bHasSpecularTex = FALSE;

	private:
		float __pad1 = 0.0f;
		float __pad2 = 0.0f;
		float __pad3 = 0.0f;

		// 16 bytes ---------------------------------
	};
	static_assert(sizeof(SGPUMaterial) == 64, "");

	// Constant Buffers ----------------------
	struct SPerFrameConstants
	{
		DirectX::SimpleMath::Matrix m_cameraViewMatrix;
		DirectX::SimpleMath::Matrix m_cameraProjectionMatrix;
		DirectX::SimpleMath::Matrix m_cameraViewProjectionMatrix;
		DirectX::SimpleMath::Matrix m_cameraInverseProjectionMatrix;
	};
	static_assert(sizeof(SPerFrameConstants) == 256, "");

	struct SPerSceneConstants
	{
		DirectX::SimpleMath::Color m_ambientColor;
		DirectX::SimpleMath::Vector2 m_screenDimensions;

	private:
		float __pad1 = 0.0f;
		float __pad2 = 0.0f;
	};
	static_assert(sizeof(SPerSceneConstants) == 32, "");

	struct SPerMaterialConstants
	{
		SGPUMaterial m_material;
	};

	struct SPerDrawConstants
	{
		DirectX::SimpleMath::Matrix m_objectToWorldMatrix;
		DirectX::SimpleMath::Matrix m_objectToViewMatrix;
		DirectX::SimpleMath::Matrix m_objectToProjectionMatrix;
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