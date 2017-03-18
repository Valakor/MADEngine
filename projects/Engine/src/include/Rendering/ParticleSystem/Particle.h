#pragma once

#include "Core/SimpleMath.h"

namespace MAD
{
	struct SCPUParticle
	{
		SCPUParticle() {}

		SCPUParticle(const Vector3& inPos, const Vector3& inVel, const Vector4& inColor, const Vector2& inSize, float inAge)
			: InitialPosVS(inPos)
			, InitialVelVS(inVel)
			, ParticleColor(inColor)
			, ParticleSize(inSize)
			, Age(inAge) {}

		Vector3 InitialPosVS;
		Vector3 InitialVelVS;
		Vector4 ParticleColor;
		Vector2 ParticleSize;
		float	Age;
	};

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