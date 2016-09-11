#include "Rendering/RenderPassDescriptor.h"
#include "Rendering/Renderer.h"
#include "Rendering/GraphicsDriver.h"
#include "Rendering/RenderPassProgram.h"
#include "Core/GameEngine.h"
#include "Misc/AssetCache.h"

namespace MAD
{
	void SRenderPassDescriptor::ApplyPassState(UGraphicsDriver& inGraphicsDriver) const
	{
		const SRenderTargetId* renderTargetIds = m_renderTargets.data();

		inGraphicsDriver.ClearDepthStencil(m_depthStencilView, true, 1.0f);

		// Apply pipeline state based on descriptor values
		inGraphicsDriver.SetRenderTargets(renderTargetIds, static_cast<int>(m_renderTargets.size()), &m_depthStencilView);

		inGraphicsDriver.SetDepthStencilState(m_depthStencilState, 0);
		
		inGraphicsDriver.SetBlendState(m_blendState);

		inGraphicsDriver.SetRasterizerState(m_rasterizerState);

		m_renderPassProgram->SetProgramActive(inGraphicsDriver);
	}

}