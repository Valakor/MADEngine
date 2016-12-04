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
	};
}