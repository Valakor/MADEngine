#pragma once

#include <EASTL/string.h>
#include <EASTL/numeric_limits.h>
#include "Rendering/RenderPassProgram.h"

namespace MAD
{
	enum class EProgramIdMask : ProgramId_t
	{
		GBuffer_Diffuse = 1 << 0,
		GBuffer_Specular = 1 << 1,
		GBuffer_Emissive = 1 << 2,
		GBuffer_OpacityMask = 1 << 3,
		GBuffer_NormalMap = 1 << 4,

		Lighting_PointLight = 1 << 5,

		INVALID = eastl::numeric_limits<ProgramId_t>::max()
	};

	enum class EMetaFlagType : uint8_t
	{
		EMetaFlagType_Usage = 0,
		EMetaFlagType_Permute,
		EMetaFlagType_Invalid
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
		EProgramIdMask PermuteIdMask;

		// ...potentially more later
	};
}