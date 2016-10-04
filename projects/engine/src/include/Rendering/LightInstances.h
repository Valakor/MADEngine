#pragma once

#include "Rendering/RenderingCommon.h"

namespace MAD
{
	struct SDirectionalLight
	{
		SGPUDirectionalLight m_gpuDirectionalLight;
		bool m_isLightEnabled;
	};

	struct SPointLight
	{
		SGPUPointLight m_gpuPointLight;
		bool m_isLightEnabled;
	};
}