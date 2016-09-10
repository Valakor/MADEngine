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

		eastl::vector<SRenderTargetId> m_renderTargets;
		SDepthStencilStateId m_depthStencilState;
		SDepthStencilId m_depthStencilView;
		SBlendStateId m_blendState;
		SRasterizerStateId m_rasterizerState;
		eastl::shared_ptr<class URenderPassProgram> m_renderPassProgram;
	};
}
