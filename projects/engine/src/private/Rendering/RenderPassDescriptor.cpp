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

		if (m_depthStencilView)
		{
			inGraphicsDriver.ClearDepthStencil(m_depthStencilView, true, 1.0f);
		}

		// Apply pipeline state based on descriptor values
		inGraphicsDriver.SetRenderTargets(renderTargetIds, static_cast<int>(m_renderTargets.size()), m_depthStencilView.IsValid() ? &m_depthStencilView : nullptr);

		if (m_depthStencilState)
		{
			inGraphicsDriver.SetDepthStencilState(m_depthStencilState, 0);
		}
		
		if (m_blendState)
		{
			inGraphicsDriver.SetBlendState(m_blendState);
		}

		if (m_rasterizerState)
		{
			inGraphicsDriver.SetRasterizerState(m_rasterizerState);
		}

		m_renderPassProgram->SetProgramActive(inGraphicsDriver);
	}

}
