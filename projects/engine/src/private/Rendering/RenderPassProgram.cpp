#include "Rendering/RenderPassProgram.h"
#include "Rendering/Renderer.h"
#include "Rendering/GraphicsDriver.h"
#include "Core/GameEngine.h"


#include <fstream>

namespace MAD
{
	EProgramIdMask URenderPassProgram::ConvertStringToPIDMask(const char* inMaskString)
	{
		if (strcmp(inMaskString, "DIFFUSE") == 0)
		{
			return EProgramIdMask::EProgramIdMask_Diffuse;
		}
		else if (strcmp(inMaskString, "SPECULAR") == 0)
		{
			return EProgramIdMask::EProgramIdMask_Specular;
		}
		else if (strcmp(inMaskString, "EMISSIVE") == 0)
		{
			return EProgramIdMask::EProgramIdMask_Emissive;
		}
		else if (strcmp(inMaskString, "NORMAL_MAP") == 0)
		{
			return EProgramIdMask::EProgramIdMask_NormalMap;
		}
		else
		{
			return EProgramIdMask::EProgramIdMask_Invalid;
		}
	}

	const char* URenderPassProgram::ConvertPIDMaskToString(EProgramIdMask inMaskId)
	{
		switch (inMaskId)
		{
		case EProgramIdMask::EProgramIdMask_Diffuse:
			return "DIFFUSE";
		case EProgramIdMask::EProgramIdMask_Specular:
			return "SPECULAR";
		case EProgramIdMask::EProgramIdMask_Emissive:
			return "EMISSIVE";
		case EProgramIdMask::EProgramIdMask_NormalMap:
			return "NORMAL_MAP";
		default:
			return "INVALID";
		}
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