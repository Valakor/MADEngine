#include "Rendering/RenderPassProgram.h"

#include <fstream>

#include "Rendering/Renderer.h"
#include "Rendering/GraphicsDriver.h"
#include "Core/GameEngine.h"
#include "Core/GameWindow.h"
#include "Misc/AssetCache.h"
#include "Misc/ProgramPermutor.h"

namespace MAD
{
	const eastl::hash_map<eastl::string, EProgramShaderType> URenderPassProgram::s_entryPointToShaderTypeMap =
	{
		{ "VS", EProgramShaderType::EProgramShaderType_VS },
		{ "GS", EProgramShaderType::EProgramShaderType_GS },
		{ "PS", EProgramShaderType::EProgramShaderType_PS }
	};

	// Uses the requested program ID to bind the correct shaders of the program to the pipeline
	// Returns whether or not the program stores a valid shader entry for the requested target program ID
	bool URenderPassProgram::SetProgramActive(class UGraphicsDriver& inGraphicsDriver, ProgramId_t inTargetProgramId) const
	{
		auto programSetFindIter = m_programPermutations.find(inTargetProgramId);

		if (programSetFindIter != m_programPermutations.cend())
		{
			// Go through all of the compiled shader byte code and create the corresponding vertex, geoemtry, and pixel shader IDs for them
			// Reason why permutation doesn't produce the shader IDs is because that is an engine level construct. If we ever move towards
			// making shader permutation generation a pre-build operation, the transition will be much smoother if the output of on-the-fly generation
			// and pre-build generation produces the same result
			const ProgramShaderTuple_t& programShaderTuple = programSetFindIter->second;
			
			// Vertex Shader
			const VertexShaderPtr_t vertexShaderPtr = GetPtrFromShaderTuple<EProgramShaderType::EProgramShaderType_VS>(programShaderTuple);
			if (vertexShaderPtr)
			{
				inGraphicsDriver.SetVertexShader(vertexShaderPtr);
			}

			// Geometry Shader (in the future, do similar for geometry shader when we are using geometry shaders)
			
			// Pixel Shader
			const PixelShaderPtr_t pixelShaderPtr = GetPtrFromShaderTuple<EProgramShaderType::EProgramShaderType_PS>(programShaderTuple);
			inGraphicsDriver.SetPixelShader(pixelShaderPtr);

			return true;
		}

		return false;
	}

	eastl::shared_ptr<URenderPassProgram> URenderPassProgram::Load(const eastl::string& inRelativePath)
	{
		if (auto cachedProgram = UAssetCache::GetCachedResource<URenderPassProgram>(inRelativePath))
		{
			return cachedProgram;
		}

		eastl::shared_ptr<URenderPassProgram> newRenderPassProgram = eastl::make_shared<URenderPassProgram>();
		UProgramPermutor::PermuteProgram(UAssetCache::GetAssetRoot() + inRelativePath, newRenderPassProgram->m_programPermutations);

		LOG(LogDefault, Log, "Loaded program `%s`\n", inRelativePath.c_str());
		UAssetCache::InsertResource<URenderPassProgram>(inRelativePath, newRenderPassProgram);
		return newRenderPassProgram;
	}

	const eastl::string& URenderPassProgram::ConvertShaderTypeToString(EProgramShaderType inShaderType)
	{
		static const eastl::string s_invalidShaderTypeString = "INVALID";

		auto shaderStringFindIter = URenderPassProgram::s_entryPointToShaderTypeMap.cbegin();
		auto shaderStringFindEndIter = URenderPassProgram::s_entryPointToShaderTypeMap.cend();

		while (shaderStringFindIter != shaderStringFindEndIter)
		{
			if (shaderStringFindIter->second == inShaderType)
			{
				return shaderStringFindIter->first;
			}

			++shaderStringFindIter;
		}

		return s_invalidShaderTypeString;
	}

	EProgramShaderType URenderPassProgram::ConvertStringToShaderType(const eastl::string& inShaderTypeString)
	{
		auto shaderTypeFindIter = URenderPassProgram::s_entryPointToShaderTypeMap.find(inShaderTypeString);

		if (shaderTypeFindIter != URenderPassProgram::s_entryPointToShaderTypeMap.cend())
		{
			return shaderTypeFindIter->second;
		}

		return EProgramShaderType::EProgramShaderType_Invalid;
	}
}
