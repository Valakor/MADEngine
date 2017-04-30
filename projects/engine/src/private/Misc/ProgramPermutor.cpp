#include "Misc/ProgramPermutor.h"

#include <fstream>
#include <iterator>
#include <string>

#include <EASTL/tuple.h>
#include <EASTL/algorithm.h>
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
	
	const eastl::hash_map<eastl::string, EProgramIdMask> UProgramPermutor::s_programIdMaskToStringMap =
	{
		{ "DIFFUSE", EProgramIdMask::GBuffer_Diffuse },
		{ "SPECULAR", EProgramIdMask::GBuffer_Specular },
		{ "EMISSIVE", EProgramIdMask::GBuffer_Emissive },
		{ "OPACITY_MASK", EProgramIdMask::GBuffer_OpacityMask },
		{ "NORMAL_MAP", EProgramIdMask::GBuffer_NormalMap },

		{ "POINT_LIGHT", EProgramIdMask::Lighting_PointLight },
		{ "DIRECTIONAL_LIGHT", EProgramIdMask::Lighting_DirectionalLight }
	};

	const eastl::hash_map<eastl::string, EMetaFlagType> UProgramPermutor::s_metaFlagStringToTypeMap =
	{
		{ "Usage", EMetaFlagType::EMetaFlagType_Usage },
		{ "Permute", EMetaFlagType::EMetaFlagType_Permute },
	};

	void UProgramPermutor::PermuteProgram(const eastl::string& inProgramFilePath, ProgramPermutations_t& outProgramPermutations, bool inShouldGenPermutationFiles)
	{
		std::ifstream programInputStream(inProgramFilePath.c_str(), std::ios::in | std::ios::ate);

		if (programInputStream.is_open())
		{
			std::string stdProgramStringBuffer;

			eastl::vector<SShaderMetaFlagInstance>						programMetaFlagInstances;
			eastl::vector<SShaderUsageDescription>						programUsageDescriptions;
			eastl::vector<SShaderPermuteDescription>						programPermutationDescriptions;

			stdProgramStringBuffer.resize(programInputStream.tellg());

			programInputStream.seekg(std::ios::beg);
			
			stdProgramStringBuffer.assign(std::istream_iterator<char>(programInputStream), std::istream_iterator<char>());

			eastl::string programStringBuffer(eastl::move(stdProgramStringBuffer.c_str()));

			// Accumulate all of the meta flag instances in the program file
			ParseProgramMetaFlags(programStringBuffer, programMetaFlagInstances);

			// Go through all meta flags and retrieve the Usage and Permute meta flag instances
			for (const auto& currentMetaFlagInst : programMetaFlagInstances)
			{
				switch (currentMetaFlagInst.MetaFlagType)
				{
					case EMetaFlagType::EMetaFlagType_Usage:
					{
						//LOG(LogProgramPermutor, Log, "Adding meta usage flag with values: %s - %s\n", currentMetaFlagInst.MetaFlagValues[0].c_str(), currentMetaFlagInst.MetaFlagValues[1].c_str());

						programUsageDescriptions.push_back({ currentMetaFlagInst.MetaFlagValues[0], currentMetaFlagInst.MetaFlagValues[1] });
						break;
					}
					case EMetaFlagType::EMetaFlagType_Permute:
					{
						EProgramIdMask permuteIdMask = UProgramPermutor::ConvertStringToPIDMask(currentMetaFlagInst.MetaFlagValues[0]);

						if (permuteIdMask != EProgramIdMask::INVALID)
						{
							//LOG(LogProgramPermutor, Log, "Adding meta permute flag with define: %s\n", currentMetaFlagInst.MetaFlagValues[0].c_str());
							programPermutationDescriptions.push_back({ permuteIdMask });
						}
						else
						{
							LOG(LogProgramPermutor, Error, "Error: Invalid permute flag: %s\n", currentMetaFlagInst.MetaFlagValues[0].c_str());

						}
						break;
					}	
				}
			}

			// The usage descriptions tell us which parts of the program to include in the shader permutation sets
			GeneratePermutations(inProgramFilePath, programUsageDescriptions, programPermutationDescriptions, outProgramPermutations, inShouldGenPermutationFiles);
		}
	}

	void UProgramPermutor::GeneratePermutations(const eastl::string& inShaderFilePath, const eastl::vector<SShaderUsageDescription>& inUsageDescriptions, const eastl::vector<SShaderPermuteDescription>& inPermuteOptions, ProgramPermutations_t& outPermutations, bool inShouldGenPermutationFiles)
	{
		UGraphicsDriver& graphicsDriver = gEngine->GetRenderer().GetGraphicsDriver();
		
		const size_t numPermuteOptions = inPermuteOptions.size();
		
		if (numPermuteOptions >= 0)
		{
			const size_t totalNumPermutations = 0x1ULL << numPermuteOptions;
			
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
						const eastl::string& permuteIdString = UProgramPermutor::ConvertPIDMaskToString(inPermuteOptions[j].PermuteIdMask);

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
				 
				programMacroDefines.push_back({ nullptr, nullptr }); // Sentinel value necessary to determine when we are at end of macro define list
				
				// For each set of usage descriptions, we need to create a new entry within the output shader permutations
				outPermutations[currentProgramId] = eastl::make_shared<UPassProgram>();

				for (const auto& currentUsageDescription : inUsageDescriptions)
				{
					compiledProgramByteCode.clear();
					
					// To limit EProgramShaderType to string conversions, we convert at the very last moment
					// Draw back, we potentially do more than we should to find out its invalid at the end, but that's okay since this will be a pre-build step eventually
					//auto shaderTypeFindIter = URenderPassProgram::s_entryPointToShaderTypeMap.find(currentUsageDescription.ShaderEntryName);
					EProgramShaderType usageShaderType = URenderPassProgram::ConvertStringToShaderType(currentUsageDescription.ShaderEntryName);

					if (usageShaderType != EProgramShaderType::EProgramShaderType_Invalid)
					{
						if (graphicsDriver.CompileShaderFromFile(inShaderFilePath, currentUsageDescription.ShaderEntryName, currentUsageDescription.ShaderModelName, compiledProgramByteCode, (programMacroDefines.size() > 1) ? programMacroDefines.data() : nullptr))
						{
							eastl::shared_ptr<UPassProgram> currentPassProgram = outPermutations[currentProgramId];

							// Compilation successful (check what type of shader it is so we can set it properly in the program ID entry)
							switch (usageShaderType)
							{
							case EProgramShaderType::EProgramShaderType_VS:
								currentPassProgram->SetVS(graphicsDriver.CreateVertexShader(compiledProgramByteCode));
								break;
							case EProgramShaderType::EProgramShaderType_GS:
								currentPassProgram->SetGS(graphicsDriver.CreateGeometryShader(compiledProgramByteCode));
								break;
							case EProgramShaderType::EProgramShaderType_PS:
								currentPassProgram->SetPS(graphicsDriver.CreatePixelShader(compiledProgramByteCode));
								break;
							}

							//LOG(LogProgramPermutor, Log, "Log: Size of compiled byte code: %d\n", compiledProgramByteCode.size());
							
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
						LOG(LogProgramPermutor, Error, "Error: Invalid or unsupported shader usage meta flag (%s, %s)\n", currentUsageDescription.ShaderEntryName.c_str(), currentUsageDescription.ShaderModelName.c_str());
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

			currentFindIndex = inShaderBufferString.find(UProgramPermutor::s_shaderMetaFlagString, currentFindIndex);
		}
	}

	// Utility functions to convert between meta flag type and string
	EProgramIdMask UProgramPermutor::ConvertStringToPIDMask(const eastl::string& inMaskString)
	{
		auto pidMaskStringFindIter = UProgramPermutor::s_programIdMaskToStringMap.find(inMaskString.c_str());

		if (pidMaskStringFindIter != UProgramPermutor::s_programIdMaskToStringMap.cend())
		{
			return pidMaskStringFindIter->second;
		}
		else
		{
			return EProgramIdMask::INVALID;
		}
	}

	const eastl::string& UProgramPermutor::ConvertPIDMaskToString(EProgramIdMask inMaskId)
	{
		static const eastl::string s_invalidMaskString = "INVALID";

		auto pidMaskFindIter = UProgramPermutor::s_programIdMaskToStringMap.cbegin();
		auto pidMaskFindEndIter = UProgramPermutor::s_programIdMaskToStringMap.cend();

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
			return EMetaFlagType::EMetaFlagType_Invalid;
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
