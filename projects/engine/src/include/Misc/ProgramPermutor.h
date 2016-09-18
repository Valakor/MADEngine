#pragma once

#include <string>

#include <EASTL/string.h>
#include <EASTL/vector.h>
#include <EASTL/hash_map.h>

#include "Rendering/RenderPassProgram.h"

namespace MAD
{
	using ProgramPermutations_t = eastl::hash_map<ProgramId_t, eastl::hash_map<eastl::string, eastl::vector<char>>>;

	class UProgramPermutor
	{
	public:
		static const char* s_shaderMetaFlagString;
	
	public:
		// Generates all the permutations of a shader based on the permutation flags that are specified within the shader file
		static void PermuteProgram(const eastl::string& inProgramFilePath, ProgramPermutations_t& outProgramPermutations);
	private:
		enum class EMetaFlagType : uint8_t
		{
			EMetaFlagType_Usage,
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
	private:
		// Utility functions
		static void ParseProgramMetaFlags(const eastl::string& inShaderBufferString, eastl::vector<SShaderMetaFlagInstance>& outMetaFlagInstances);
		static void GeneratePermutations(const eastl::string& inShaderFilePath, const eastl::vector<SShaderUsageDescription>& inUsageDescriptions, const eastl::vector<SShaderPermuteDescription>& inPermuteOptions, ProgramPermutations_t& outPermutations);
		static void GenerateProgramPermutationFile(const eastl::string& inOutputFilePath, const eastl::vector<char>& inCompiledByteCode, const SShaderUsageDescription& inTargetUsage, ProgramId_t inTargetProgramID, const eastl::string& inTargetProgramStringDesc);
		static EMetaFlagType ConvertStringToFlagType(const eastl::string& inMetaFlagString);
	};
}
