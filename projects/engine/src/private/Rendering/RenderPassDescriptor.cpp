#include "Rendering/RenderPassDescriptor.h"
#include "Rendering/Renderer.h"
#include "Rendering/GraphicsDriver.h"

namespace MAD
{
	void SRenderPassDescriptor::ApplyPassState(UGraphicsDriver& inGraphicsDriver) const
	{
		// Apply pipeline state based on descriptor values
		const RenderTargetPtr_t* renderTargetPtrs = m_renderTargets.data();

		inGraphicsDriver.SetRenderTargets(renderTargetPtrs, static_cast<int>(m_renderTargets.size()), (m_depthStencilView.Get() != nullptr) ? m_depthStencilView : nullptr);

		inGraphicsDriver.SetDepthStencilState(m_depthStencilState, 0);
		
		inGraphicsDriver.SetBlendState(m_blendState);

		if (m_rasterizerState) inGraphicsDriver.SetRasterizerState(m_rasterizerState);
	}
}
