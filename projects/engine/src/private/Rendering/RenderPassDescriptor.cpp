#include "Rendering/RenderPassDescriptor.h"
#include "Rendering/Renderer.h"
#include "Rendering/GraphicsDriver.h"

namespace MAD
{
	void SRenderPassDescriptor::ApplyPassState(UGraphicsDriver& inGraphicsDriver) const
	{
		// Apply pipeline state based on descriptor values
		const SRenderTargetId* renderTargetIds = m_renderTargets.data();
		inGraphicsDriver.SetRenderTargets(renderTargetIds, static_cast<int>(m_renderTargets.size()), m_depthStencilView.IsValid() ? &m_depthStencilView : nullptr);

		inGraphicsDriver.SetDepthStencilState(m_depthStencilState, 0);
		
		inGraphicsDriver.SetBlendState(m_blendState);

		if (m_rasterizerState.IsValid()) inGraphicsDriver.SetRasterizerState(m_rasterizerState);
	}
}
