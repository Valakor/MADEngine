#pragma once

#include <EASTL/shared_ptr.h>
#include <EASTL/string.h>
#include <EASTL/hash_map.h>
#include <EASTL/vector.h>
#include <EASTL/functional.h>
#include <EASTL/tuple.h>
#include <EASTL/type_traits.h>

#include "Rendering/GraphicsDriverTypes.h"

namespace MAD
{
	// For now, the order that the enum shader types are specified should be the same order the shader ID's are specified in the tuple
	enum class EProgramShaderType : uint8_t
	{
		EProgramShaderType_VS = 0,
		EProgramShaderType_GS,
		EProgramShaderType_PS,
		EProgramShaderType_Invalid
	};

	using ProgramId_t = uint64_t;
	using ProgramShaderTuple_t = eastl::tuple<SVertexShaderId, SGeometryShaderId, SPixelShaderId>;
	using ProgramPermutations_t = eastl::hash_map<ProgramId_t, ProgramShaderTuple_t>;

	// Utility template getter function for shader tuple
	template <EProgramShaderType ShaderType>
	auto GetIdFromShaderTuple(ProgramShaderTuple_t& inShaderTuple) -> decltype(eastl::get<static_cast<eastl::underlying_type<EProgramShaderType>::type>(ShaderType)>(inShaderTuple));

	class URenderPassProgram
	{
	public:
		static const eastl::string& ConvertShaderTypeToString(EProgramShaderType inShaderType);
		static EProgramShaderType ConvertStringToShaderType(const eastl::string& inShaderTypeString);
	public:
		void SetProgramActive(class UGraphicsDriver& inGraphicsDriver) const;
		bool SetProgramActive(class UGraphicsDriver& inGraphicsDriver, ProgramId_t inTargetProgramId);
	private:
		friend class UAssetCache;
		static eastl::shared_ptr<URenderPassProgram> Load(const eastl::string& inPath);
	private:
		static const eastl::hash_map<eastl::string, EProgramShaderType> s_entryPointToShaderTypeMap;
	private:
		// For now, we're only going to use a program with (potentially) only a vertex shader and pixel shader
		// Will probably support an additional geometry shader
		
		SVertexShaderId m_vertexShader;
		SPixelShaderId m_pixelShader;

		ProgramPermutations_t m_programPermutations;
	};

	template <EProgramShaderType ShaderType>
	auto GetIdFromShaderTuple(ProgramShaderTuple_t& inShaderTuple) -> decltype(eastl::get<static_cast<eastl::underlying_type<EProgramShaderType>::type>(ShaderType)>(inShaderTuple))
	{
		return eastl::get<static_cast<eastl::underlying_type<EProgramShaderType>::type>(ShaderType)>(inShaderTuple);
	}
}
