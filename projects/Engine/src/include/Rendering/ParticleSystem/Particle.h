#pragma once

#include "Core/SimpleMath.h"

namespace MAD
{
	struct SGPUParticle
	{
		Vector3 InitialPosVS;
		float	__pad1;
		Vector3 InitialVelVS;
		float	__pad2;
		Vector4 ParticleColor;
		Vector2 ParticleSize;
		float	__pad3[2];
	};

	static_assert(sizeof(SGPUParticle) % sizeof(Vector4) == 0, "Size of SGPUParticle needs to be 16 byte aligned");
}