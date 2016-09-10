#pragma once

#include "Rendering/RenderingCommon.h"

namespace MAD
{
	struct SPointLight
	{
		SGPUPointLight m_gpuPointLight;
		bool m_isEnabled;
	};
}