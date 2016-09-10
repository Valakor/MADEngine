#include "Rendering/RenderPassProgram.h"
#include "Rendering/Renderer.h"
#include "Rendering/GraphicsDriver.h"
#include "Core/GameEngine.h"

namespace MAD
{

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