#include "Misc/ProgramPermutor.h"

#include <fstream>
#include <iterator>
#include <regex>

#include <EASTL/algorithm.h>
#include <EASTL/functional.h>

#include "Rendering/Renderer.h"
#include "Core/GameEngine.h"
#include "Rendering/GraphicsDriver.h"

#include "Misc/Logging.h"

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogProgramPermutor);

	const char* UProgramPermutor::s_shaderMetaFlagString = "//=>:(";

	void UProgramPermutor::PermuteProgram(const eastl::string& inProgramFilePath, ProgramPermutations_t& outProgramPermutations)
	{
		(void)outProgramPermutations;

		std::string stdProgramFilePath(inProgramFilePath.c_str()); // Have to make copy of string because EASTL doesn't have streams
		std::ifstream programInputStream(stdProgramFilePath, std::ios::in | std::ios::ate);

		if (programInputStream.is_open())
		{
			std::string programStringBuffer;
			eastl::vector<UProgramPermutor::SShaderMetaFlagInstance>						programMetaFlagInstances;
			eastl::vector<UProgramPermutor::SShaderUsageDescription>						programUsageDescriptions;
			eastl::vector<UProgramPermutor::SShaderPermuteDescription>						programPermutationDescriptions;

			programStringBuffer.reserve(programInputStream.tellg());

			programInputStream.seekg(std::ios::beg);

			programStringBuffer.assign(std::istream_iterator<char>(programInputStream), std::istream_iterator<char>());

			// Accumulate all of the meta flag instances in the program file
			ParseProgramMetaFlags(programStringBuffer, programMetaFlagInstances);

			// Go through all meta flags and retrieve the Usage and Permute meta flag instances
			// TODO: Maybe move the logic to generate specific structs by type (i.e usage description generation logic separated out from here)
			for (const auto& currentMetaFlagInst : programMetaFlagInstances)
			{
				switch (currentMetaFlagInst.MetaFlagType)
				{
				case EMetaFlagType::EMetaFlagType_Usage:
					LOG(LogProgramPermutor, Log, "Adding meta usage flag with values: %s - %s\n", currentMetaFlagInst.MetaFlagValues[0].c_str(), currentMetaFlagInst.MetaFlagValues[1].c_str());
					programUsageDescriptions.push_back({ currentMetaFlagInst.MetaFlagValues[0], currentMetaFlagInst.MetaFlagValues[1] });
					break;
				case EMetaFlagType::EMetaFlagType_Permute:
					EProgramIdMask permuteIdMask = URenderPassProgram::ConvertStringToPIDMask(currentMetaFlagInst.MetaFlagValues[0].c_str());

					if (permuteIdMask != EProgramIdMask::EProgramIdMask_Invalid)
					{
						LOG(LogProgramPermutor, Log, "Adding meta permute flag with define: %s\n", currentMetaFlagInst.MetaFlagValues[0].c_str());
						programPermutationDescriptions.push_back({ permuteIdMask });
					}
					else
					{
						LOG(LogProgramPermutor, Error, "Error: Invalid permute flag: %s\n", currentMetaFlagInst.MetaFlagValues[0].c_str());

					}
					break;
				}
			}

			// The usage descriptions tell us which parts of the program to include in the shader permutation sets
			GeneratePermutations(inProgramFilePath, programUsageDescriptions, programPermutationDescriptions, outProgramPermutations);
			
			// Output: eastl::hash_map<ProgramId_t, eastl::hash_map<EntryName, eastl::vector<char>>
			// Each program ID will be calculated using bit values that are specified by convention
		}
	}

	void UProgramPermutor::GeneratePermutations(const eastl::string& inShaderFilePath, const eastl::vector<SShaderUsageDescription>& inUsageDescriptions, const eastl::vector<SShaderPermuteDescription>& inPermuteOptions, ProgramPermutations_t& outPermutations)
	{
		UGraphicsDriver& graphicsDriver = gEngine->GetRenderer().GetGraphicsDriver();
		
		if (inPermuteOptions.size() > 0)
		{
			size_t numPermuteOptions = inPermuteOptions.size();
			size_t totalNumPermutations = 0x1ULL << numPermuteOptions;
			
			for (size_t i = 0; i < totalNumPermutations; ++i)
			{
				ProgramId_t currentProgramId = 0;
				eastl::vector<D3D_SHADER_MACRO> programMacroDefines;
				eastl::vector<char> compiledProgramByteCode;
				
				programMacroDefines.reserve(inPermuteOptions.size());
				
				// Find the bits that are set and mask the associated bit mask with the program ID
				for (size_t j = 0; j < numPermuteOptions; ++j)
				{
					if ((i & (0x1ULL << j)) != 0)
					{
						currentProgramId |= static_cast<ProgramId_t>(inPermuteOptions[j].PermuteIdMask);
						programMacroDefines.push_back({ URenderPassProgram::ConvertPIDMaskToString(inPermuteOptions[j].PermuteIdMask), "1" });
					}
				}

				programMacroDefines.push_back({ nullptr, nullptr }); // Sentinel value needed to determine when we are at end of macro list

				for (const auto& currentUsageDescription : inUsageDescriptions)
				{
					// For the current program ID, we want to create the compiled byte code for the current usage
					if (graphicsDriver.CompileShaderFromFile(inShaderFilePath, currentUsageDescription.ShaderEntryName, currentUsageDescription.ShaderModelName, compiledProgramByteCode, nullptr, (programMacroDefines.size() > 1) ? programMacroDefines.data() : nullptr))
					{
						// Compilation successful
						outPermutations[currentProgramId][currentUsageDescription.ShaderEntryName] = eastl::move(compiledProgramByteCode);
					}
					else
					{
						LOG(LogProgramPermutor, Error, "Error: Couldn't compile shader for usage of (%s-%s) and program ID of %d\n", currentUsageDescription.ShaderEntryName.c_str(), currentUsageDescription.ShaderModelName.c_str(), currentProgramId);
					}
				}
			}
		}
	}

	UProgramPermutor::EMetaFlagType UProgramPermutor::ConvertStringToFlagType(const eastl::string& inMetaFlagString)
	{
		if (inMetaFlagString == "Usage")
		{
			return EMetaFlagType::EMetaFlagType_Usage;
		}
		else if (inMetaFlagString == "Permute")
		{
			return EMetaFlagType::EMetaFlagType_Permute;
		}
		else
		{
			return EMetaFlagType::EMetaFlagType_Invalid;
		}
	}
	// Parses the entire shader string buffer to match instances of the program meta flag
	// Format of the shader usage flags: "!!>:(Flag Type, <Values, sep. by comma>)"
	// Flag Type can be either Usage or Permute
	void UProgramPermutor::ParseProgramMetaFlags(const std::string& inShaderBufferString, eastl::vector<SShaderMetaFlagInstance>& outMetaFlagInstances)
	{
		// Find all occurrences of shader meta flag string
		size_t currentFindIndex = inShaderBufferString.find(s_shaderMetaFlagString, 0);

		outMetaFlagInstances.clear();

		while (currentFindIndex != eastl::string::npos)
		{
			eastl::string metaFlagInstanceString;

			currentFindIndex += strlen(s_shaderMetaFlagString);

			while (inShaderBufferString[currentFindIndex] != ')')
			{
				metaFlagInstanceString += inShaderBufferString[currentFindIndex];

				++currentFindIndex;
			}

			++currentFindIndex; // Skip past the ending parentheses

			if (metaFlagInstanceString.empty())
			{
				LOG(LogProgramPermutor, Warning, "Warning: Shader usage instance #%d has an empty description pair!", outMetaFlagInstances.size());
				return;
			}

			eastl::vector<eastl::string> metaFlagSubstrings;
			SShaderMetaFlagInstance currentMetaFlagInstance;

			// Split the meta flag string by coma
			size_t currentCommaIndex = 0;
			size_t nextCommaIndex = metaFlagInstanceString.find(',', currentCommaIndex);

			while (nextCommaIndex != eastl::string::npos)
			{
				eastl::string flagValueString = metaFlagInstanceString.substr(currentCommaIndex, (nextCommaIndex - currentCommaIndex));

				metaFlagSubstrings.emplace_back(flagValueString);

				currentCommaIndex = nextCommaIndex + 1;

				nextCommaIndex = metaFlagInstanceString.find(',', currentCommaIndex);

				if (nextCommaIndex == eastl::string::npos)
				{
					eastl::string lastFlagValueString = metaFlagInstanceString.substr(currentCommaIndex, (nextCommaIndex - currentCommaIndex));

					// On the last iteration, we need to add the last string
					metaFlagSubstrings.emplace_back(lastFlagValueString);
				}
			}
			
			// Flag string count sanity check
			if (metaFlagSubstrings.size() == 0)
			{
				LOG(LogProgramPermutor, Warning, "Warning: Shader usage instance #%d has more than one separation commas!", outMetaFlagInstances.size());
				return;
			}
			
			// First substring is the flag type
			currentMetaFlagInstance.MetaFlagType = ConvertStringToFlagType(metaFlagSubstrings.front());

			if (currentMetaFlagInstance.MetaFlagType == EMetaFlagType::EMetaFlagType_Invalid)
			{
				LOG(LogProgramPermutor, Warning, "Warning: Shader usage instance #%d has an invalid meta flag type!", outMetaFlagInstances.size());
				return;
			}
			
			// Rest of the substrings are the parameter values (if applicable)
			
			for (size_t i = 1; i < metaFlagSubstrings.size(); ++i)
			{
				currentMetaFlagInstance.MetaFlagValues.emplace_back(metaFlagSubstrings[i]);
			}

			outMetaFlagInstances.emplace_back(currentMetaFlagInstance);

			currentFindIndex = inShaderBufferString.find(s_shaderMetaFlagString, currentFindIndex);
		}
	}


}