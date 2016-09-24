#include "Rendering/RenderPassProgram.h"
#include "Rendering/Renderer.h"
#include "Rendering/GraphicsDriver.h"
#include "Core/GameEngine.h"


#include <fstream>

namespace MAD
{
	const eastl::hash_map<eastl::string, EProgramIdMask> URenderPassProgram::s_programIdMaskToStringMap =
	{
		{ "DIFFUSE", EProgramIdMask::EProgramIdMask_Diffuse },
		{ "SPECULAR", EProgramIdMask::EProgramIdMask_Specular },
		{ "EMISSIVE", EProgramIdMask::EProgramIdMask_Emissive },
		{ "NORMAL_MAP", EProgramIdMask::EProgramIdMask_NormalMap }
	};

	EProgramIdMask URenderPassProgram::ConvertStringToPIDMask(const eastl::string& inMaskString)
	{
		auto pidMaskStringFindIter = URenderPassProgram::s_programIdMaskToStringMap.find(inMaskString);

		if (pidMaskStringFindIter != URenderPassProgram::s_programIdMaskToStringMap.cend())
		{
			return pidMaskStringFindIter->second;
		}
		else
		{
			return EProgramIdMask::EProgramIdMask_Invalid;
		}
	}

	eastl::string URenderPassProgram::ConvertPIDMaskToString(EProgramIdMask inMaskId)
	{
		auto pidMaskFindIter = URenderPassProgram::s_programIdMaskToStringMap.cbegin();
		auto pidMaskFindEndIter = URenderPassProgram::s_programIdMaskToStringMap.cend();

		while (pidMaskFindIter != pidMaskFindEndIter)
		{
			if (pidMaskFindIter->second == inMaskId)
			{
				return pidMaskFindIter->first;
			}
		}

		return "INVALID";
	}

	void URenderPassProgram::SetProgramActive(UGraphicsDriver& inGraphicsDriver) const
	{
		inGraphicsDriver.SetVertexShader(m_vertexShader);
		inGraphicsDriver.SetPixelShader(m_pixelShader);
	}

	eastl::shared_ptr<URenderPassProgram> URenderPassProgram::Load(const eastl::string& inPath)
	{
		eastl::shared_ptr<URenderPassProgram> newRenderPassProgram = eastl::make_shared<URenderPassProgram>();

		auto& graphicsDevice = gEngine->GetRenderer().GetGraphicsDriver();

		eastl::vector<char> compiledVertexShaderByteCode;
		eastl::vector<char> compiledPixelShaderByteCode;
	
		// Every program MUST always have a valid vertex shader
		if (!graphicsDevice.CompileShaderFromFile(inPath, "VS", "vs_5_0", compiledVertexShaderByteCode))
		{
			return nullptr;
		}
		else
		{
			newRenderPassProgram->m_vertexShader = graphicsDevice.CreateVertexShader(compiledVertexShaderByteCode);
		}

		// However, not all programs require a bound pixel shader
		if (graphicsDevice.CompileShaderFromFile(inPath, "PS", "ps_5_0", compiledPixelShaderByteCode))
		{
			newRenderPassProgram->m_pixelShader = graphicsDevice.CreatePixelShader(compiledPixelShaderByteCode);
		}

		return newRenderPassProgram;
	}

}