#pragma once

#include "DirectXTK/SimpleMath.h"

namespace MAD
{
	struct SCPUPointLightInstance
	{
		DirectX::SimpleMath::Vector3 m_lightPosition;
		float m_lightRadius;
		DirectX::SimpleMath::Color m_lightColor;
		float m_lightIntensity;
		bool m_isEnabled;
	};

	struct SGPUPointLightInstance
	{
		DirectX::SimpleMath::Vector3 m_lightPosition;
		float m_lightRadius;
		DirectX::SimpleMath::Color m_lightColor;
		float m_lightIntensity;
	};
}