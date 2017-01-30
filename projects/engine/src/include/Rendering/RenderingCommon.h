#pragma once

#include "Core/SimpleMath.h"

#include <EASTL/type_traits.h>

namespace MAD
{
#ifdef _DEBUG
#define DX_HRESULT(DXOp, ErrorMsg)				\
	do											\
	{											\
		HRESULT hr = (DXOp);					\
												\
		if (!SUCCEEDED(hr))						\
		{										\
			MAD_ASSERT_DESC(false, ErrorMsg);	\
		}										\
	} while (0)
#else
#define DX_HRESULT(DXOp, ErrorMsg) (DXOp)
#endif

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
		OpacityMask,
		NormalMap,

		// ------------- Defined by renderer -------------------
		LightingBuffer,
		DiffuseBuffer,
		NormalBuffer,
		SpecularBuffer,
		DepthBuffer,
		ShadowMap,
		ShadowCube,

		FontMap,

		MAX
	};
	DECLARE_SLOT_TO_INTEGRAL(ETextureSlot)

	enum class ERenderTargetSlot
	{
		LightingBuffer = 0,
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
		ShadowMap,

		MAX
	};
	DECLARE_SLOT_TO_INTEGRAL(ESamplerSlot)

	enum class EVertexBufferSlot : uint8_t
	{
		Position = 0,
		Normal,
		Tangent,
		UV,

		MAX
	};
	DECLARE_SLOT_TO_INTEGRAL(EVertexBufferSlot);

#undef DECLARE_SLOT_TO_INTEGRAL

	// Lights -------------------------------
	struct SGPUPointLight
	{
		Vector3 m_lightPosition;
		float m_lightIntensity;
		Color m_lightColor;
		float m_lightInnerRadius;
		float m_lightOuterRadius;
	private:
		float __pad1 = 0.0f;
		float __pad2 = 0.0f;
	public:
		Matrix m_viewProjectionMatrix;
	public:
		static SGPUPointLight Lerp(const SGPUPointLight& a, const SGPUPointLight& b, float t);
	};
	static_assert(sizeof(SGPUPointLight) == 112, "");

	struct SGPUDirectionalLight
	{
		Vector3 m_lightDirection;
		float m_lightIntensity;
		Color m_lightColor;

		Matrix m_viewProjectionMatrix;

	public:
		static SGPUDirectionalLight Lerp(const SGPUDirectionalLight& a, const SGPUDirectionalLight& b, float t);

		void CalculateViewProjection(float inOrthoWidth, float inOrthoHeight, float inNear, float inFar);
	};
	static_assert(sizeof(SGPUDirectionalLight) == 96, "");

	// Materials --------------------------------
	struct SGPUMaterial
	{
	public:
		Vector3 m_diffuseColor;
		float m_opacity = 1.0f;
		// 16 bytes ---------------------------------

		Vector3 m_specularColor;
		float m_specularPower = 1.0f;
		// 16 bytes ---------------------------------

		Vector3 m_emissiveColor;
	private:
		float __pad2 = 0.0f;
		// 16 bytes ---------------------------------
	};
	static_assert(sizeof(SGPUMaterial) == 48, "");

	// Constant Buffers ----------------------
	struct SPerFrameConstants
	{
		Matrix m_cameraViewMatrix;
		Matrix m_cameraProjectionMatrix;
		Matrix m_cameraViewProjectionMatrix;
		Matrix m_cameraInverseViewMatrix;
		Matrix m_cameraInverseProjectionMatrix;

		float m_cameraNearPlane;
		float m_cameraFarPlane;

		float m_cameraExposure;

	private:
		float __pad2 = 0.0f;
	};
	static_assert(sizeof(SPerFrameConstants) == 336, "");

	struct SPerSceneConstants
	{
		Color m_ambientColor;
		Vector2 m_screenDimensions;

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
		Matrix m_objectToWorldMatrix;
		Matrix m_objectToViewMatrix;
		Matrix m_objectToProjectionMatrix;
	};

	struct SPerPointLightConstants
	{
		SGPUPointLight m_pointLight;

		Matrix m_pointLightVPMatrices[6];
	};

	struct SPerDirectionalLightConstants
	{
		SGPUDirectionalLight m_directionalLight;
	};
}