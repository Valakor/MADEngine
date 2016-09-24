#pragma once

#include <EASTL/shared_ptr.h>
#include <EASTL/string.h>
#include <EASTL/hash_map.h>
#include <EASTL/vector.h>
#include <EASTL/functional.h>

#include "Rendering/GraphicsDriverTypes.h"

namespace MAD
{
	enum EProgramShaderType : uint8_t
	{
		EProgramShaderType_VS = 0,
		EProgramShaderType_GS,
		EProgramShaderType_PS,
	};

}

// Why do we have to do this...? Sigh, you would think it would just use the integral value type (look into this)
namespace eastl
{
	template <>
	struct eastl::hash<MAD::EProgramShaderType>
	{
		size_t operator()(MAD::EProgramShaderType inShaderType) const
		{
			return static_cast<size_t>(inShaderType);
		}
	};

	template <>
	struct eastl::equal_to<MAD::EProgramShaderType>
	{
		bool operator()(MAD::EProgramShaderType inLeftType, MAD::EProgramShaderType inRightType) const
		{
			return inLeftType == inRightType;
		}
	};
}

namespace MAD
{
	using ProgramId_t = uint64_t;

	// TODO: Maybe makes more sense to be withint he ProgramPermutor files?
	enum class EProgramIdMask : ProgramId_t
	{
		EProgramIdMask_Diffuse = 1 << 0,
		EProgramIdMask_Specular = 1 << 1,
		EProgramIdMask_Emissive = 1 << 2,
		EProgramIdMask_NormalMap = 1 << 3,
		EProgramIdMask_Invalid
	};

	using ProgramPermutationContainer_t = eastl::hash_map<EProgramShaderType, eastl::vector<char>>;
	using ProgramPermutations_t = eastl::hash_map<ProgramId_t, ProgramPermutationContainer_t>;

	class URenderPassProgram
	{
	public:
		static EProgramIdMask ConvertStringToPIDMask(const eastl::string& inMaskString);
		static eastl::string ConvertPIDMaskToString(EProgramIdMask inMaskId);
	public:
		void SetProgramActive(class UGraphicsDriver& inGraphicsDriver) const;
	private:
		friend class UAssetCache;
		static eastl::shared_ptr<URenderPassProgram> Load(const eastl::string& inPath);

		static const eastl::hash_map<eastl::string, EProgramIdMask> s_programIdMaskToStringMap;
	private:
		// For now, we're only going to use a program with (potentially) only a vertex shader and pixel shader
		// Will probably support an additional geometry shader
		
		SVertexShaderId m_vertexShader;
		SPixelShaderId m_pixelShader;

		ProgramPermutations_t m_programPermutations;
	};
}
