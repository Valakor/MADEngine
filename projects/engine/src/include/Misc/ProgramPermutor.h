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
		static void PermuteProgram(const eastl::string& inProgramFilePath, ProgramPermutations_t& outProgramPermutations, bool inShouldGenPermutationFiles = true);
	private:
		static const eastl::string s_shaderMetaFlagString;
		static const eastl::hash_map<eastl::string, EMetaFlagType> s_metaFlagStringToTypeMap;
		static const eastl::hash_map<eastl::string, ProgramIdMask_t> s_stringToProgramIdMaskMap;
	private:
		// Parsing and generation functions
		static void ParseProgramMetaFlags(const eastl::string& inShaderBufferString, eastl::vector<SShaderMetaFlagInstance>& outMetaFlagInstances);
		
		static void GenerateProgramIdSet(const eastl::vector<SShaderPermuteGroupDescription>& inPermuteGroups, eastl::vector<ProgramId_t>& outPermutedProgramIds);
		static void GenerateProgramIdsForGroup(const SShaderPermuteGroupDescription& inPermuteGroupDesc, eastl::vector<ProgramId_t>& outPermutedProgramIds);

		static void GeneratePermutations(const eastl::string& inShaderFilePath, const eastl::vector<SShaderUsageDescription>& inUsageDescriptions, const eastl::vector<SShaderPermuteDescription>& inPermuteOptions, const eastl::vector<ProgramId_t>& inTargetProgramIds, bool inShouldGenPermutationFiles, ProgramPermutations_t& outPermutations);

		static void GenerateProgramPermutationFile(const eastl::string& inOutputFilePath, const eastl::string& inProgramFileName, const eastl::vector<char>& inCompiledByteCode, const SShaderUsageDescription& inTargetUsage, ProgramId_t inTargetProgramID, const eastl::string& inTargetProgramStringDesc);
		
		// Utility functions
		static uint32_t ParsePermuteGroupFlags(const SShaderMetaFlagInstance& inCurrentFlagInst, size_t& outNumGroupFlags);
		static void		ParsePermuteGroupParameters(const SShaderMetaFlagInstance& inCurrentMetaFlagInst, eastl::vector<SShaderPermuteDescription>& inOutPermuteOptions, eastl::vector<SShaderPermuteGroupDescription>& inOutPermuteGroups);

		static ProgramIdMask_t ConvertStringToPIDMask(const eastl::string& inMaskString);
		static const eastl::string& ConvertPIDMaskToString(ProgramIdMask_t inMaskId);
		static EMetaFlagType ConvertStringToFlagType(const eastl::string& inMetaFlagString);
		static const eastl::string& ConvertFlagTypeToString(EMetaFlagType inMetaFlagType);
	};
}
