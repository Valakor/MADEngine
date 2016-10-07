#pragma once

#include <EASTL/string.h>
#include <EASTL/numeric_limits.h>
#include "Rendering/RenderPassProgram.h"

namespace MAD
{
	namespace EIdMask
	{
		constexpr ProgramIdMask_t ID_MASK_BIT(uint8_t bitNum)
		{
			return 0x1ULL << bitNum;
		}

		const ProgramIdMask_t INVALID_MASK_ID = eastl::numeric_limits<ProgramIdMask_t>::max();

		enum EGBufferIdMask : ProgramIdMask_t
		{
			GBuffer_Diffuse = ID_MASK_BIT(0),
			GBuffer_Specular = ID_MASK_BIT(1),
			GBuffer_Emissive = ID_MASK_BIT(2),
			GBuffer_OpacityMask = ID_MASK_BIT(3),
			GBuffer_NormalMap = ID_MASK_BIT(4),
		};

		enum ELightingIdMask : ProgramIdMask_t
		{
			Lighting_PointLight = ID_MASK_BIT(0),
			Lighting_DirectionalLight = ID_MASK_BIT(1),
			Lighting_SpotLight = ID_MASK_BIT(2),
		};
	}

	enum class EMetaFlagType : uint8_t
	{
		Usage = 0,
		Permute,
		INVALID
	};

	struct SShaderMetaFlagInstance
	{
		EMetaFlagType MetaFlagType;
		eastl::vector<eastl::string> MetaFlagValues;
	};

	struct SShaderUsageDescription
	{
		eastl::string ShaderEntryName;
		eastl::string ShaderModelName;
	};

	struct SShaderPermuteDescription
	{
		ProgramIdMask_t PermuteIdMask;
		eastl::string PermuteIdMaskName;
	};

	enum EPermuteGroupFlags : uint32_t
	{
		EPermuteGroupFlags_Mutual = 1 << 0,
	};

	struct SShaderPermuteGroupDescription
	{
		eastl::vector<SShaderPermuteDescription> GroupPermuteOptions;
		uint32_t GroupFlags;
	};
}