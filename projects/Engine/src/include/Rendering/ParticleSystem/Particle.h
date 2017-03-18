#pragma once

#include "Core/SimpleMath.h"

namespace MAD
{
	struct SCPUParticle
	{
		SCPUParticle() {}

		SCPUParticle(const Vector3& inPos, const Vector3& inVel, const Vector4& inColor, const Vector2& inSize, float inAge, float inDuration)
			: InitialPosVS(inPos)
			, InitialVelVS(inVel)
			, ParticleColor(inColor)
			, ParticleSize(inSize)
			, Age(inAge)
			, Duration(inDuration) {}

		Vector3 InitialPosVS;
		Vector3 InitialVelVS;
		Vector4 ParticleColor;
		Vector2 ParticleSize;
		float	Age;
		float	Duration;
	};
}