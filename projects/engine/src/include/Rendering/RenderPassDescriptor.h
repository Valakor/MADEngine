#pragma once

#include "Rendering/GraphicsDriverTypes.h"

#include <EASTL/vector.h>
#include <EASTL/shared_ptr.h>
#include <EASTL/string.h>

namespace MAD
{
	struct SRenderPassDescriptor
	{
		void ApplyPassState(class UGraphicsDriver& inGraphicsDriver) const;

		eastl::array<RenderTargetPtr_t, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> m_renderTargets;
		DepthStencilStatePtr_t m_depthStencilState;
		DepthStencilPtr_t m_depthStencilView;
		BlendStatePtr_t m_blendState;
		RasterizerStatePtr_t m_rasterizerState;
		eastl::shared_ptr<class URenderPassProgram> m_renderPassProgram;
	};
}
