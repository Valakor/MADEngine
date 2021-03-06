#pragma once

#include <EASTL/vector.h>
#include <EASTL/hash_map.h>

#include "Misc/ProgramPermutorInfoTypes.h"

namespace MAD
{
	class UProgramPermutor
	{
	public:
		// Generates all the permutations of a shader based on the permutation flags that are specified within the shader file
		static void PermuteProgram(const eastl::string& inProgramFilePath, ProgramPermutations_t& outProgramPermutations, bool inShouldGenPermutationFiles = false);
	private:
		static const eastl::string s_shaderMetaFlagString;
		static const eastl::hash_map<eastl::string, EMetaFlagType> s_metaFlagStringToTypeMap;
		static const eastl::hash_map<eastl::string, EProgramIdMask> s_programIdMaskToStringMap;
	private:
		// Parsing and generation functions
		static void ParseProgramMetaFlags(const eastl::string& inShaderBufferString, eastl::vector<SShaderMetaFlagInstance>& outMetaFlagInstances);
		static void GeneratePermutations(const eastl::string& inShaderFilePath, const eastl::vector<SShaderUsageDescription>& inUsageDescriptions, const eastl::vector<SShaderPermuteDescription>& inPermuteOptions, ProgramPermutations_t& outPermutations, bool inShouldGenPermutationFiles);
		static void GenerateProgramPermutationFile(const eastl::string& inOutputFilePath, const eastl::string& inProgramFileName, const eastl::vector<char>& inCompiledByteCode, const SShaderUsageDescription& inTargetUsage, ProgramId_t inTargetProgramID, const eastl::string& inTargetProgramStringDesc);
		
		// Utility functions
		static EProgramIdMask ConvertStringToPIDMask(const eastl::string& inMaskString);
		static const eastl::string& ConvertPIDMaskToString(EProgramIdMask inMaskId);
		static EMetaFlagType ConvertStringToFlagType(const eastl::string& inMetaFlagString);
		static const eastl::string& ConvertFlagTypeToString(EMetaFlagType inMetaFlagType);
	};
}
