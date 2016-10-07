#include "Misc/ProgramPermutor.h"

#include <fstream>
#include <iterator>
#include <string>

#include <EASTL/tuple.h>
#include <EASTL/algorithm.h>
#include <EASTL/sort.h>
#include <EASTL/functional.h>

#include "Rendering/Renderer.h"
#include "Core/GameEngine.h"
#include "Rendering/GraphicsDriver.h"
#include "Rendering/GraphicsDriverTypes.h"

#include "Misc/Logging.h"

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogProgramPermutor);

	/** Program Permutation Constants -------------------------- */
	const eastl::string UProgramPermutor::s_shaderMetaFlagString = "//=>:(";

	const eastl::hash_map<eastl::string, ProgramIdMask_t> UProgramPermutor::s_stringToProgramIdMaskMap =
	{
		{ "DIFFUSE", EIdMask::GBuffer_Diffuse },
		{ "SPECULAR", EIdMask::GBuffer_Specular },
		{ "EMISSIVE", EIdMask::GBuffer_Emissive },
		{ "OPACITY_MASK", EIdMask::GBuffer_OpacityMask },
		{ "NORMAL_MAP", EIdMask::GBuffer_NormalMap },

		{ "POINT_LIGHT", EIdMask::Lighting_PointLight },
		{ "DIRECTIONAL_LIGHT", EIdMask::Lighting_DirectionalLight },
		{ "SPOT_LIGHT", EIdMask::Lighting_SpotLight }
	};

	const eastl::hash_map<eastl::string, EMetaFlagType> UProgramPermutor::s_metaFlagStringToTypeMap =
	{
		{ "Usage", EMetaFlagType::Usage },
		{ "Permute", EMetaFlagType::Permute },
	};

	void UProgramPermutor::PermuteProgram(const eastl::string& inProgramFilePath, ProgramPermutations_t& outProgramPermutations, bool inShouldGenPermutationFiles)
	{
		std::ifstream programInputStream(inProgramFilePath.c_str(), std::ios::in | std::ios::ate);

		if (programInputStream.is_open())
		{
			std::string stdProgramStringBuffer;

			eastl::vector<SShaderMetaFlagInstance>				programMetaFlagInstances;
			eastl::vector<SShaderUsageDescription>				programUsageDescriptions;
			eastl::vector<SShaderPermuteDescription>			programPermutationDescriptions;
			eastl::vector<SShaderPermuteGroupDescription>		programPermutationGroupDescs; // Permutation groups

			stdProgramStringBuffer.resize(programInputStream.tellg());

			programInputStream.seekg(std::ios::beg);
			
			stdProgramStringBuffer.assign(std::istream_iterator<char>(programInputStream), std::istream_iterator<char>());

			eastl::string programStringBuffer(eastl::move(stdProgramStringBuffer.c_str()));

			// Accumulate all of the meta flag instances in the program file
			ParseProgramMetaFlags(programStringBuffer, programMetaFlagInstances);

			// The value of the meta flag type encodes the priority in which the meta flag instance will be processed (sort by meta flag type value)
			eastl::insertion_sort(programMetaFlagInstances.begin(), programMetaFlagInstances.end(), [](const SShaderMetaFlagInstance& inLeftFlagInst, const SShaderMetaFlagInstance& inRightFlagInst)
			{
				return inLeftFlagInst.MetaFlagType < inRightFlagInst.MetaFlagType;
			});

			// Go through all meta flags and retrieve the Usage and Permute meta flag instances
			for (const auto& currentMetaFlagInst : programMetaFlagInstances)
			{
				switch (currentMetaFlagInst.MetaFlagType)
				{
					case EMetaFlagType::Usage:
					{
						LOG(LogProgramPermutor, Log, "Adding meta usage flag with values: %s - %s\n", currentMetaFlagInst.MetaFlagValues[0].c_str(), currentMetaFlagInst.MetaFlagValues[1].c_str());

						programUsageDescriptions.push_back({ currentMetaFlagInst.MetaFlagValues[0], currentMetaFlagInst.MetaFlagValues[1] });
						
						break;
					}
					case EMetaFlagType::Permute:
					{
						ProgramIdMask_t currentPermuteIdMask = UProgramPermutor::ConvertStringToPIDMask(currentMetaFlagInst.MetaFlagValues[0]);

						// Make sure they're using a valid permute name
						if (currentPermuteIdMask != EIdMask::INVALID_MASK_ID)
						{
							// Parses the parameters (options and values) of the current permute group, along with creating the list of permute options for everything
							ParsePermuteGroupParameters(currentMetaFlagInst, programPermutationDescriptions, programPermutationGroupDescs);
						}
						else
						{
							LOG(LogProgramPermutor, Error, "Error: Invalid permute flag: %s\n", currentMetaFlagInst.MetaFlagValues[0].c_str());
						}

						break;
					}
				}
			}

			// TEMP: Create a list of program Ids from 0 to 0x1ULL << num permute options
			eastl::vector<ProgramId_t> targetProgramIds;

			// Generate the ProgramId set to create permutations for
			GenerateProgramIdSet(programPermutationGroupDescs, targetProgramIds);

			// The usage descriptions tell us which parts of the program to include in the shader permutation sets
			GeneratePermutations(inProgramFilePath, programUsageDescriptions, programPermutationDescriptions, targetProgramIds, inShouldGenPermutationFiles, outProgramPermutations);
		}
	}

	void UProgramPermutor::GenerateProgramIdSet(const eastl::vector<SShaderPermuteGroupDescription>& inPermuteGroups, eastl::vector<ProgramId_t>& outPermutedProgramIds)
	{
		// Go through each of the permute groups and generate program Ids for them

		for (const auto& currentPermuteGroup : inPermuteGroups)
		{
			GenerateProgramIdsForGroup(currentPermuteGroup, outPermutedProgramIds);
		}
	}

	void UProgramPermutor::GenerateProgramIdsForGroup(const SShaderPermuteGroupDescription& inPermuteGroupDesc, eastl::vector<ProgramId_t>& outPermutedProgramIds)
	{
		// For the current input permute group, we need to check the group flags to see if we need to perform any special actions
		const size_t maxNumPermutations = 0x1ULL << inPermuteGroupDesc.GroupPermuteOptions.size();

		outPermutedProgramIds.reserve(maxNumPermutations);

		for (size_t i = 0; i < maxNumPermutations; ++i)
		{
			outPermutedProgramIds.push_back(i);
		}
	}

	void UProgramPermutor::GeneratePermutations(const eastl::string& inShaderFilePath, const eastl::vector<SShaderUsageDescription>& inUsageDescriptions, const eastl::vector<SShaderPermuteDescription>& inPermuteOptions, const eastl::vector<ProgramId_t>& inTargetProgramIds, bool inShouldGenPermutationFiles, ProgramPermutations_t& outPermutations)
	{
		UGraphicsDriver& graphicsDriver = gEngine->GetRenderer().GetGraphicsDriver();

		const size_t numPermuteOptions = inPermuteOptions.size();

		if (numPermuteOptions >= 0)
		{
			const size_t totalNumProgramIds = inTargetProgramIds.size();

			for (size_t i = 0; i < totalNumProgramIds; ++i)
			{
				ProgramId_t currentProgramId = inTargetProgramIds[i];
				eastl::vector<D3D_SHADER_MACRO> programMacroDefines;
				eastl::vector<char> compiledProgramByteCode;
				eastl::string currentProgramIdString;

				programMacroDefines.reserve(numPermuteOptions);

				// Find the bits that are set and mask the associated bit mask with the program ID
				if (numPermuteOptions > 0)
				{
					for (size_t j = 0; j < numPermuteOptions; ++j)
					{
						currentProgramIdString += '[';

						if ((currentProgramId & inPermuteOptions[j].PermuteIdMask) != 0)
						{
							currentProgramIdString += '+';

							programMacroDefines.push_back({ inPermuteOptions[j].PermuteIdMaskName.c_str(), "1" });
						}
						else
						{
							currentProgramIdString += "-";
						}

						currentProgramIdString += inPermuteOptions[j].PermuteIdMaskName.c_str();
						currentProgramIdString += ']';
					}
				}
				else
				{
					currentProgramIdString += "[NO PERMUTE OPTIONS]";
				}

				programMacroDefines.push_back({ nullptr, nullptr }); // Sentinel value necessary to determine when we are at end of macro define list

				for (const auto& currentUsageDescription : inUsageDescriptions)
				{
					compiledProgramByteCode.clear();

					// To limit EProgramShaderType to string conversions, we convert at the very last moment
					// Draw back, we potentially do more than we should to find out its invalid at the end, but that's okay since this will be a pre-build step eventually
					EProgramShaderType usageShaderType = URenderPassProgram::ConvertStringToShaderType(currentUsageDescription.ShaderEntryName);

					if (usageShaderType != EProgramShaderType::EProgramShaderType_Invalid)
					{
						if (graphicsDriver.CompileShaderFromFile(inShaderFilePath, currentUsageDescription.ShaderEntryName, currentUsageDescription.ShaderModelName, compiledProgramByteCode, (programMacroDefines.size() > 1) ? programMacroDefines.data() : nullptr))
						{
							ProgramShaderTuple_t& currentProgramIDTuple = outPermutations[currentProgramId];

							// Compilation successful (check what type of shader it is so we can set it properly in the program ID entry)
							switch (usageShaderType)
							{
							case EProgramShaderType::EProgramShaderType_VS:
								SetIdToShaderTuple<EProgramShaderType::EProgramShaderType_VS>(currentProgramIDTuple, graphicsDriver.CreateVertexShader(compiledProgramByteCode));
								break;
							case EProgramShaderType::EProgramShaderType_GS:
								//SetIdToShaderTuple<EProgramShaderType::EProgramShaderType_GS>(currentProgramIDTuple, /* create geometry shader here eventually */
								break;
							case EProgramShaderType::EProgramShaderType_PS:
								SetIdToShaderTuple<EProgramShaderType::EProgramShaderType_PS>(currentProgramIDTuple, graphicsDriver.CreatePixelShader(compiledProgramByteCode));
								break;
							}

							LOG(LogProgramPermutor, Log, "Log: [%s] Size of compiled byte code: %d\n", currentUsageDescription.ShaderEntryName.c_str(), compiledProgramByteCode.size());

							if (inShouldGenPermutationFiles)
							{
								// Stores the generated shader permutation file in the same directory as the shader file that it is permuting
								eastl::string shaderFilePath = inShaderFilePath;
								size_t shaderFileNameBeginIndex = shaderFilePath.find_last_of('\\') + 1;
								size_t shaderFileNameEndIndex = shaderFilePath.find_last_of('.');
								eastl::string shaderFileName(shaderFilePath.substr(shaderFileNameBeginIndex, shaderFileNameEndIndex - shaderFileNameBeginIndex));

								shaderFilePath.erase(shaderFilePath.find_last_of('\\'), eastl::string::npos);

								GenerateProgramPermutationFile(shaderFilePath, shaderFileName, compiledProgramByteCode, currentUsageDescription, currentProgramId, currentProgramIdString);
							}
						}
						else
						{
							LOG(LogProgramPermutor, Error, "Error: Couldn't compile shader for usage of (%s-%s) and program ID of %d\n", currentUsageDescription.ShaderEntryName.c_str(), currentUsageDescription.ShaderModelName.c_str(), currentProgramId);
						}
					}
					else
					{
						LOG(LogProgramPermutor, Error, "Error: Invalid or unsupported shader usage meta flag (%s, %s)\n", currentUsageDescription.ShaderEntryName, currentUsageDescription.ShaderModelName);
					}
				}
			}
		}
	}

	void UProgramPermutor::GenerateProgramPermutationFile(const eastl::string& inOutputFilePath, const eastl::string& inProgramFileName, const eastl::vector<char>& inCompiledByteCode, const SShaderUsageDescription& inTargetUsage, ProgramId_t inTargetProgramID, const eastl::string& inTargetProgramStringDesc)
	{
		eastl::string outputFilePath = inOutputFilePath;
		
		outputFilePath += '\\';
		outputFilePath += inProgramFileName;
		outputFilePath += '_';
		outputFilePath += std::to_string(inTargetProgramID).c_str(); // Linker error if I try to use eastl::to_string (??) seems to not support using uint64_t
		outputFilePath += '_';
		outputFilePath += inTargetProgramStringDesc;
		outputFilePath += '_';
		outputFilePath += inTargetUsage.ShaderEntryName;
		outputFilePath += '-';
		outputFilePath += inTargetUsage.ShaderModelName;
		outputFilePath += ".cso";

		 std::ofstream outputProgramFileStream(outputFilePath.c_str(), std::ios::out | std::ios::binary);

		if (outputProgramFileStream.is_open())
		{
			outputProgramFileStream.write(inCompiledByteCode.data(), inCompiledByteCode.size());

			outputProgramFileStream.flush();
		}
	}

	uint32_t UProgramPermutor::ParsePermuteGroupFlags(const SShaderMetaFlagInstance& inCurrentFlagInst, size_t& outNumGroupFlags)
	{
		uint32_t outputGroupFlags = 0;

		// TODO: For now, we only support the "Mutual" option
		if (inCurrentFlagInst.MetaFlagValues[1] == "Mutual")
		{
			outputGroupFlags |= static_cast<uint32_t>(EPermuteGroupFlags::EPermuteGroupFlags_Mutual);
			++outNumGroupFlags;
		}

		return outputGroupFlags;
	}

	void UProgramPermutor::ParsePermuteGroupParameters(const SShaderMetaFlagInstance& inCurrentMetaFlagInst, eastl::vector<SShaderPermuteDescription>& inOutPermuteOptions, eastl::vector<SShaderPermuteGroupDescription>& inOutPermuteGroups)
	{
		const size_t numMetaFlagValues = inCurrentMetaFlagInst.MetaFlagValues.size();
		SShaderPermuteGroupDescription currentGroupDesc;
		size_t numGroupFlags = 0;

		// Before checking the permute options of the group, we need to check if there are any flags to set for the group
		currentGroupDesc.GroupFlags = ParsePermuteGroupFlags(inCurrentMetaFlagInst, numGroupFlags);

		// After checking the flags, we can use the rest of the arguments as permute options for the group (starting at numGroupFlags + 1 until end of the meta flag values)
		for (size_t i = numGroupFlags + 1; i < numMetaFlagValues; ++i)
		{
			ProgramIdMask_t currentMetaFlagValueMask = UProgramPermutor::ConvertStringToPIDMask(inCurrentMetaFlagInst.MetaFlagValues[i]);

			if (currentMetaFlagValueMask != EIdMask::INVALID_MASK_ID)
			{
				LOG(LogProgramPermutor, Log, "Adding meta permute flag with define: %s\n", inCurrentMetaFlagInst.MetaFlagValues[i].c_str());
				currentGroupDesc.GroupPermuteOptions.push_back({ currentMetaFlagValueMask, inCurrentMetaFlagInst.MetaFlagValues[i] });
			}
			else
			{
				LOG(LogProgramPermutor, Error, "Error: Invalid permute option (%s) at index %d\n", inCurrentMetaFlagInst.MetaFlagValues[i].c_str(), i);
			}
		}

		inOutPermuteGroups.emplace_back(currentGroupDesc);
		inOutPermuteOptions.insert(inOutPermuteOptions.end(), currentGroupDesc.GroupPermuteOptions.cbegin(), currentGroupDesc.GroupPermuteOptions.cend());
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

			currentFindIndex += UProgramPermutor::s_shaderMetaFlagString.length();

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

			if (currentMetaFlagInstance.MetaFlagType == EMetaFlagType::INVALID)
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

			currentFindIndex = inShaderBufferString.find(UProgramPermutor::s_shaderMetaFlagString, currentFindIndex);
		}
	}

	// Utility functions to convert between meta flag type and string
	ProgramIdMask_t UProgramPermutor::ConvertStringToPIDMask(const eastl::string& inMaskString)
	{
		auto pidMaskStringFindIter = UProgramPermutor::s_stringToProgramIdMaskMap.find(inMaskString.c_str());

		if (pidMaskStringFindIter != UProgramPermutor::s_stringToProgramIdMaskMap.cend())
		{
			return pidMaskStringFindIter->second;
		}
		else
		{
			return EIdMask::INVALID_MASK_ID;
		}
	}

	const eastl::string& UProgramPermutor::ConvertPIDMaskToString(ProgramIdMask_t inMaskId)
	{
		static const eastl::string s_invalidMaskString = "INVALID";

		auto pidMaskFindIter = UProgramPermutor::s_stringToProgramIdMaskMap.cbegin();
		auto pidMaskFindEndIter = UProgramPermutor::s_stringToProgramIdMaskMap.cend();

		while (pidMaskFindIter != pidMaskFindEndIter)
		{
			if (pidMaskFindIter->second == inMaskId)
			{
				return pidMaskFindIter->first;
			}

			++pidMaskFindIter;
		}

		return s_invalidMaskString;
	}

	EMetaFlagType UProgramPermutor::ConvertStringToFlagType(const eastl::string& inMetaFlagString)
	{
		// TODO: Transform input meta flag string to lower case so we can be case insensitive (we probably want this? maybe not)
		auto metaFlagFindIter = UProgramPermutor::s_metaFlagStringToTypeMap.find(inMetaFlagString);

		if (metaFlagFindIter != UProgramPermutor::s_metaFlagStringToTypeMap.cend())
		{
			return metaFlagFindIter->second;
		}
		else
		{
			return EMetaFlagType::INVALID;
		}
	}


	const eastl::string& UProgramPermutor::ConvertFlagTypeToString(EMetaFlagType inMetaFlagType)
	{
		static const eastl::string s_invalidFlagTypeString = "Invalid";

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

		return s_invalidFlagTypeString;
	}
}
