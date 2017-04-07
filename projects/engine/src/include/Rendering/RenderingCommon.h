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

#define DECLARE_ENUM_TO_INTEGRAL(SlotEnum) \
	inline constexpr eastl::underlying_type<SlotEnum>::type AsIntegral(SlotEnum e) \
	{ \
		return static_cast<eastl::underlying_type<SlotEnum>::type>(e); \
	}

#define DECLARE_ENUM_BITWISE_OPERATOR(EnumType)																															\
	inline EnumType operator | (EnumType leftEnum, EnumType rightEnum)																									\
	{																																									\
		using TypeAlias = eastl::underlying_type<EnumType>::type;																										\
																																										\
		return static_cast<EnumType>(static_cast<TypeAlias>(leftEnum) | static_cast<TypeAlias>(rightEnum));																\
	}																																									\
																																										\
	inline EnumType& operator|=(EnumType& targetEnum, EnumType sourceEnum)																								\
	{																																									\
		using TypeAlias = eastl::underlying_type<EnumType>::type;																										\
																																										\
		targetEnum = EnumType(static_cast<TypeAlias>(targetEnum) | static_cast<TypeAlias>(sourceEnum));																	\
																																										\
		return targetEnum;																																				\
	}


	// DirectX11 Enumeration Wrappers -----------
	enum class EPrimitiveTopology : eastl::underlying_type<D3D11_PRIMITIVE_TOPOLOGY>::type
	{
		Undefined = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED,
		PointList = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST,
		LineList = D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
		LineStrip = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP,
		TriangleList = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		TriangleStrip = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
	};

	enum class EResourceUsage : eastl::underlying_type<D3D11_USAGE>::type
	{
		Default = D3D11_USAGE_DEFAULT,
		Immutable = D3D11_USAGE_IMMUTABLE,
		Dynamic = D3D11_USAGE_DYNAMIC,
		Staging = D3D11_USAGE_STAGING
	};

	enum class ECPUAccess : eastl::underlying_type<D3D11_CPU_ACCESS_FLAG>::type
	{
		None = 0,
		Read = D3D11_CPU_ACCESS_READ,
		Write = D3D11_CPU_ACCESS_WRITE
	};

	enum class EBindFlag : eastl::underlying_type<D3D11_BIND_FLAG>::type
	{
		VertexBuffer = D3D11_BIND_VERTEX_BUFFER,
		IndexBuffer = D3D11_BIND_INDEX_BUFFER,
		ConstantBuffer = D3D11_BIND_CONSTANT_BUFFER,
		ShaderResource = D3D11_BIND_SHADER_RESOURCE,
		StreamOutput = D3D11_BIND_STREAM_OUTPUT,
		RenderTarget = D3D11_BIND_RENDER_TARGET,
		DepthStencil = D3D11_BIND_DEPTH_STENCIL,
		UnorderedAccess = D3D11_BIND_UNORDERED_ACCESS,
		Decoder = D3D11_BIND_DECODER,
		VideoDecoder = D3D11_BIND_VIDEO_ENCODER
	};

	enum class EFillMode : eastl::underlying_type<D3D11_FILL_MODE>::type
	{
		WireFrame = D3D11_FILL_WIREFRAME,
		Solid = D3D11_FILL_SOLID
	};

	enum class ECullMode : eastl::underlying_type<D3D11_CULL_MODE>::type
	{
		None = D3D11_CULL_NONE,
		Front = D3D11_CULL_FRONT,
		Back = D3D11_CULL_BACK
	};

	enum class EBlendFactor : eastl::underlying_type<D3D11_BLEND>::type
	{
		Zero = D3D11_BLEND_ZERO,
		One = D3D11_BLEND_ONE,
		SrcColor = D3D11_BLEND_SRC_COLOR,
		InvSrcColor = D3D11_BLEND_INV_SRC_COLOR,
		SrcAlpha = D3D11_BLEND_SRC_ALPHA,
		InvSrcAlpha = D3D11_BLEND_INV_SRC_ALPHA,
		DestAlpha = D3D11_BLEND_DEST_ALPHA,
		InvDestAlpha = D3D11_BLEND_INV_DEST_ALPHA,
		DestColor = D3D11_BLEND_DEST_COLOR,
		InvDestColor = D3D11_BLEND_INV_DEST_COLOR,
		SrcAlpha_Sat = D3D11_BLEND_SRC_ALPHA_SAT,
		BlendFactor = D3D11_BLEND_BLEND_FACTOR,
		InvBlendFactor = D3D11_BLEND_INV_BLEND_FACTOR,
		Src1Color = D3D11_BLEND_SRC1_COLOR,
		InvSrc1Color = D3D11_BLEND_INV_SRC1_COLOR,
		Src1Alpha = D3D11_BLEND_SRC1_ALPHA,
		InvSrc1Alpha = D3D11_BLEND_INV_SRC1_ALPHA
	};

	enum class EBlendOp : eastl::underlying_type<D3D11_BLEND_OP>::type
	{
		Add = D3D11_BLEND_OP_ADD,
		Subtract = D3D11_BLEND_OP_SUBTRACT,
		RevSubtract = D3D11_BLEND_OP_REV_SUBTRACT,
		Min = D3D11_BLEND_OP_MIN,
		Max = D3D11_BLEND_OP_MAX
	};

	enum class EComparisonFunc : eastl::underlying_type<D3D11_COMPARISON_FUNC>::type
	{
		Never = D3D11_COMPARISON_NEVER,
		Less = D3D11_COMPARISON_LESS,
		Equal = D3D11_COMPARISON_EQUAL,
		LessEqual = D3D11_COMPARISON_LESS_EQUAL,
		Greater = D3D11_COMPARISON_GREATER,
		NotEqual = D3D11_COMPARISON_NOT_EQUAL,
		GreaterEqual = D3D11_COMPARISON_GREATER_EQUAL,
		Always = D3D11_COMPARISON_ALWAYS
	};

	enum class EDepthWriteMask : eastl::underlying_type<D3D11_DEPTH_WRITE_MASK>::type
	{
		Zero = D3D11_DEPTH_WRITE_MASK_ZERO,
		All = D3D11_DEPTH_WRITE_MASK_ALL
	};

	enum class EResourceMap : eastl::underlying_type<D3D11_MAP>::type
	{
		Read = D3D11_MAP_READ,
		Write = D3D11_MAP_WRITE,
		ReadWrite = D3D11_MAP_READ_WRITE,
		WriteDiscard = D3D11_MAP_WRITE_DISCARD,
		WriteNoOverwrite = D3D11_MAP_WRITE_NO_OVERWRITE
	};

	DECLARE_ENUM_TO_INTEGRAL(EPrimitiveTopology)
	DECLARE_ENUM_TO_INTEGRAL(EResourceUsage)
	DECLARE_ENUM_TO_INTEGRAL(ECPUAccess)
	DECLARE_ENUM_TO_INTEGRAL(EBindFlag)

	DECLARE_ENUM_BITWISE_OPERATOR(ECPUAccess)
	DECLARE_ENUM_BITWISE_OPERATOR(EBindFlag)
	DECLARE_ENUM_BITWISE_OPERATOR(EResourceMap)

	// Runtime Pipeline Enums ---------------
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
	DECLARE_ENUM_TO_INTEGRAL(EConstantBufferSlot)

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
		ReflectionBuffer,
		CubeMap,

		MAX
	};
	DECLARE_ENUM_TO_INTEGRAL(ETextureSlot)

	enum class ERenderTargetSlot
	{
		LightingBuffer = 0,
		DiffuseBuffer,
		NormalBuffer,
		SpecularBuffer,
		ReflectionBuffer,

		MAX
	};
	DECLARE_ENUM_TO_INTEGRAL(ERenderTargetSlot);

	enum class ESamplerSlot
	{
		Point = 0,
		Linear,
		Trilinear,
		Anisotropic,
		ShadowMap,

		MAX
	};
	DECLARE_ENUM_TO_INTEGRAL(ESamplerSlot)

	using VertexBufferSlotType_t = uint8_t;

	// TODO Changed back to classed enum, but added static_casts where this is used in functions (change parameter to uint8_t instead of EVertexBufferSlot)
	enum EVertexBufferSlot : VertexBufferSlotType_t
	{
		Position = 0,
		Normal,
		Tangent,
		UV,

		MAX
	};

	DECLARE_ENUM_TO_INTEGRAL(EVertexBufferSlot);

	enum class EParticleVertexBufferSlot : VertexBufferSlotType_t
	{
		InitialPos = 0,
		InitialVel,
		Color,
		Size,
		Age,

		MAX
	};

	DECLARE_ENUM_TO_INTEGRAL(EParticleVertexBufferSlot);

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
		float m_reflectivity;
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

		float m_gameTime;
		float m_frameTime;
	private:
		float __pad1 = 0.0f;
		float __pad2 = 0.0f;
		float __pad3 = 0.0f;
	};
	static_assert(sizeof(SPerFrameConstants) == 352, "");

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