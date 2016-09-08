#pragma once

#include "DirectXTK/SimpleMath.h"

namespace MAD
{
	struct SCPUDirectionalLightInstance
	{
		DirectX::SimpleMath::Vector3 m_lightDirection;
		DirectX::SimpleMath::Color m_lightColor;
		float m_lightIntensity;
		bool m_isLightEnabled;
	};

	struct SGPUDirectionalLightInstance
	{
		DirectX::SimpleMath::Vector3 m_lightDirection;
		DirectX::SimpleMath::Color m_lightColor;
		float m_lightIntensity;
	};
}