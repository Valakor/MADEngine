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
	auto GetIdFromShaderTuple(const ProgramShaderTuple_t& inShaderTuple) -> decltype(eastl::get<static_cast<eastl::underlying_type<EProgramShaderType>::type>(ShaderType)>(inShaderTuple));

	template <EProgramShaderType ShaderType, typename ShaderIdType>
	void SetIdToShaderTuple(ProgramShaderTuple_t& inShaderTuple, const ShaderIdType& inShaderId);

	class URenderPassProgram
	{
	public:
		/*
		* Loads a program at the given path. The path should be relative to the assets root directory. Internally uses a cache
		* to ensure programs are only loaded and compiled once
		*/
		static eastl::shared_ptr<URenderPassProgram> Load(const eastl::string& inRelativePath);

		static const eastl::string& ConvertShaderTypeToString(EProgramShaderType inShaderType);
		static EProgramShaderType ConvertStringToShaderType(const eastl::string& inShaderTypeString);
	public:
		bool SetProgramActive(class UGraphicsDriver& inGraphicsDriver, ProgramId_t inTargetProgramId) const;
	private:
		static const eastl::hash_map<eastl::string, EProgramShaderType> s_entryPointToShaderTypeMap;
	private:
		// For now, we're only going to use a program with (potentially) only a vertex shader and pixel shader
		// Will probably support an additional geometry shader
		ProgramPermutations_t m_programPermutations;
	};

	template <EProgramShaderType ShaderType>
	auto GetIdFromShaderTuple(const ProgramShaderTuple_t& inShaderTuple) -> decltype(eastl::get<static_cast<eastl::underlying_type<EProgramShaderType>::type>(ShaderType)>(inShaderTuple))
	{
		return eastl::get<static_cast<eastl::underlying_type<EProgramShaderType>::type>(ShaderType)>(inShaderTuple);
	}

	template <EProgramShaderType ShaderType, typename ShaderIdType>
	void SetIdToShaderTuple(ProgramShaderTuple_t& inShaderTuple, const ShaderIdType& inShaderId)
	{
		eastl::get<static_cast<eastl::underlying_type<EProgramShaderType>::type>(ShaderType)>(inShaderTuple) = inShaderId;
	}
}
