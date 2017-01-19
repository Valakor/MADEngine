#pragma once

#include "Core/SimpleMath.h"

namespace MAD
{
	struct SCameraInstance
	{
		// Assume perspective for now
		float m_verticalFOV;
		float m_nearPlaneDistance;
		float m_farPlaneDistance;
		float m_exposure;
		ULinearTransform m_transform;

		SCameraInstance()
		{
			m_verticalFOV = ConvertToRadians(60.0f);
			m_nearPlaneDistance = 3.0f;
			m_farPlaneDistance = 10000.0f;
			m_exposure = 1.0f;
		}
	};
}