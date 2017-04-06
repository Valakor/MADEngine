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

	class UPassProgram
	{
	public:
		UPassProgram();

		void SetVS(const VertexShaderPtr_t& inVS) { m_vs = inVS; }
		void SetGS(const GeometryShaderPtr_t& inGS) { m_gs = inGS; }
		void SetPS(const PixelShaderPtr_t& inPS) { m_ps = inPS; }

		void BindToPipeline(class UGraphicsDriver& inGraphicsDriver);
	private:
		VertexShaderPtr_t m_vs;
		GeometryShaderPtr_t m_gs;
		PixelShaderPtr_t m_ps;
	};

	using ProgramId_t = uint64_t;
	using ProgramPermutations_t = eastl::hash_map<ProgramId_t, eastl::shared_ptr<UPassProgram>>;

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
		ProgramPermutations_t m_programPermutations;
	};
}
