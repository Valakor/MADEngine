#pragma once

#include "Core/SimpleMath.h"

namespace MAD
{
	struct SCameraInstance
	{
		float m_verticalFOV;
		float m_nearPlaneDistance;
		float m_farPlaneDistance;
		DirectX::SimpleMath::Matrix m_viewMatrix;
		DirectX::SimpleMath::Matrix m_projectionMatrix;
		DirectX::SimpleMath::Matrix m_viewProjectionMatrix;
	};
}