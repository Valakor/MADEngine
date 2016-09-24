#include "Misc/ProgramPermutor.h"

#include <fstream>
#include <iterator>
#include <string>

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
	
	const eastl::hash_map<eastl::string, EProgramShaderType> UProgramPermutor::s_entryPointToShaderTypeMap =
	{
		{ "VS", EProgramShaderType::EProgramShaderType_VS },
		{ "GS", EProgramShaderType::EProgramShaderType_GS },
		{ "PS", EProgramShaderType::EProgramShaderType_PS }
	};

	const eastl::hash_map<eastl::string, UProgramPermutor::EMetaFlagType> UProgramPermutor::s_metaFlagStringToTypeMap =
	{
		{ "Usage", UProgramPermutor::EMetaFlagType::EMetaFlagType_Usage },
		{ "Permute", UProgramPermutor::EMetaFlagType::EMetaFlagType_Permute },
	};

	void UProgramPermutor::PermuteProgram(const eastl::string& inProgramFilePath, ProgramPermutations_t& outProgramPermutations)
	{
		std::ifstream programInputStream(inProgramFilePath.c_str(), std::ios::in | std::ios::ate);

		if (programInputStream.is_open())
		{
			std::string stdProgramStringBuffer;
			eastl::vector<UProgramPermutor::SShaderMetaFlagInstance>						programMetaFlagInstances;
			eastl::vector<UProgramPermutor::SShaderUsageDescription>						programUsageDescriptions;
			eastl::vector<UProgramPermutor::SShaderPermuteDescription>						programPermutationDescriptions;

			stdProgramStringBuffer.reserve(programInputStream.tellg());

			programInputStream.seekg(std::ios::beg);
			
			stdProgramStringBuffer.assign(std::istream_iterator<char>(programInputStream), std::istream_iterator<char>());

			eastl::string programStringBuffer(eastl::move(stdProgramStringBuffer.c_str()));

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
		
		if (inPermuteOptions.size() >= 0)
		{
			size_t numPermuteOptions = inPermuteOptions.size();
			size_t totalNumPermutations = 0x1ULL << numPermuteOptions;
			
			for (size_t i = 0; i < totalNumPermutations; ++i)
			{
				ProgramId_t currentProgramId = 0;
				eastl::vector<D3D_SHADER_MACRO> programMacroDefines;
				eastl::vector<char> compiledProgramByteCode;
				eastl::string currentProgramIdString;

				programMacroDefines.reserve(inPermuteOptions.size());
				
				// Find the bits that are set and mask the associated bit mask with the program ID
				if (numPermuteOptions > 0)
				{
					for (size_t j = 0; j < numPermuteOptions; ++j)
					{
						const eastl::string permuteIdString = URenderPassProgram::ConvertPIDMaskToString(inPermuteOptions[j].PermuteIdMask);

						currentProgramIdString += '[';

						if ((i & (0x1ULL << j)) != 0)
						{
							currentProgramId |= static_cast<ProgramId_t>(inPermuteOptions[j].PermuteIdMask);

							currentProgramIdString += '+';

							programMacroDefines.push_back({ permuteIdString.c_str(), "1" });
						}
						else
						{
							currentProgramIdString += "-";
						}

						currentProgramIdString += permuteIdString;
						currentProgramIdString += ']';
					}
				}
				else
				{
					currentProgramIdString += "[NO PERMUTE OPTIONS]";
				}
				 
				 programMacroDefines.push_back({ nullptr, nullptr }); // Sentinel value needed to determine when we are at end of macro list

				for (const auto& currentUsageDescription : inUsageDescriptions)
				{
					compiledProgramByteCode.clear();
					
					// For the current program ID, we want to create the compiled byte code for the current usage
					if (graphicsDriver.CompileShaderFromFile(inShaderFilePath, currentUsageDescription.ShaderEntryName, currentUsageDescription.ShaderModelName, compiledProgramByteCode, (programMacroDefines.size() > 1) ? programMacroDefines.data() : nullptr))
					{
						// Compilation successful
						eastl::string shaderFilePath = inShaderFilePath;

						shaderFilePath.erase(shaderFilePath.find_last_of('\\'), eastl::string::npos);

						LOG(LogProgramPermutor, Log, "Log: Size of compiled byte code: %d\n", compiledProgramByteCode.size());

						GenerateProgramPermutationFile(shaderFilePath, compiledProgramByteCode, currentUsageDescription, currentProgramId, currentProgramIdString);

						// To limit EProgramShaderType to string conversions, we convert at the very last moment
						// Draw back, we potentially do more than we should to find out its invalid at the end, but that's since this will be a pre-build step eventually
						auto shaderTypeFindIter = UProgramPermutor::s_entryPointToShaderTypeMap.find(currentUsageDescription.ShaderEntryName);

						if (shaderTypeFindIter != UProgramPermutor::s_entryPointToShaderTypeMap.cend())
						{
							outPermutations[currentProgramId][shaderTypeFindIter->second] = eastl::move(compiledProgramByteCode);
						}
						else
						{
							LOG(LogProgramPermutor, Error, "Error: Invalid or unsupported shader usage meta flag (%s, %s)\n", currentUsageDescription.ShaderEntryName, currentUsageDescription.ShaderModelName);
						}
					}
					else
					{
						LOG(LogProgramPermutor, Error, "Error: Couldn't compile shader for usage of (%s-%s) and program ID of %d\n", currentUsageDescription.ShaderEntryName.c_str(), currentUsageDescription.ShaderModelName.c_str(), currentProgramId);
					}
				}
			}
		}
	}

	void UProgramPermutor::GenerateProgramPermutationFile(const eastl::string& inOutputFilePath, const eastl::vector<char>& inCompiledByteCode, const SShaderUsageDescription& inTargetUsage, ProgramId_t inTargetProgramID, const eastl::string& inTargetProgramStringDesc)
	{
		std::string outputFilePath = inOutputFilePath.c_str();

		outputFilePath += '\\';
		outputFilePath += std::to_string(inTargetProgramID);
		outputFilePath += '_';
		outputFilePath += inTargetProgramStringDesc.c_str();
		outputFilePath += '_';
		outputFilePath += inTargetUsage.ShaderEntryName.c_str();
		outputFilePath += '-';
		outputFilePath += inTargetUsage.ShaderModelName.c_str();
		outputFilePath += ".cso";

		 std::ofstream outputProgramFileStream(outputFilePath, std::ios::out | std::ios::binary);

		if (outputProgramFileStream.is_open())
		{
			for (auto currentChar : inCompiledByteCode)
			{
				outputProgramFileStream << currentChar;
			}

			 outputProgramFileStream.flush();
		}
	}

	// Utility functions to convert between meta flag type and string

	UProgramPermutor::EMetaFlagType UProgramPermutor::ConvertStringToFlagType(const eastl::string& inMetaFlagString)
	{
		// TODO: Transform input meta flag string to lower case so we can be case insensitive (we probably want this? maybe not)
		auto metaFlagFindIter = UProgramPermutor::s_metaFlagStringToTypeMap.find(inMetaFlagString);
		
		if (metaFlagFindIter != UProgramPermutor::s_metaFlagStringToTypeMap.cend())
		{
			return metaFlagFindIter->second;
		}
		else
		{
			return EMetaFlagType::EMetaFlagType_Invalid;
		}
	}


	eastl::string UProgramPermutor::ConvertFlagTypeToString(EMetaFlagType inMetaFlagType)
	{
		auto metaFlagFindIter = UProgramPermutor::s_metaFlagStringToTypeMap.cbegin();
		auto metaFlagEndIter = UProgramPermutor::s_metaFlagStringToTypeMap.cend();

		while (metaFlagFindIter != metaFlagEndIter)
		{
			if (metaFlagFindIter->second == inMetaFlagType)
			{
				return metaFlagFindIter->first;
			}

			++metaFlagFindIter;
		}

		return "Invalid";
	}

	// Parses the entire shader string buffer to match instances of the program meta flag
	// Format of the shader usage flags: "!!>:(Flag Type, <Values, sep. by comma>)"
	// Flag Type can be either Usage or Permute
	void UProgramPermutor::ParseProgramMetaFlags(const eastl::string& inShaderBufferString, eastl::vector<SShaderMetaFlagInstance>& outMetaFlagInstances)
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